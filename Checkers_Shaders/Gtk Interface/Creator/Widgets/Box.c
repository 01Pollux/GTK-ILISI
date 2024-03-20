#include "Box.h"

/*
 * Prototype: void BoxDesc_Init(BoxDesc* Desc);
 * Entree: Pointeur description du Box .
 * Sortie: Initialisation des valeurs de la structure BoxDesc.
 */
void BoxDesc_Init(BoxDesc* Desc)
{
    // Initialisation des parametres de la structure
    Desc->Fixed = FALSE;
    Desc->Spacing = 5;
    Desc->Orientation = GTK_ORIENTATION_VERTICAL;
}

/*
 * Prototype: GtkWidget* Box_Create(BoxDesc* Desc);
 * Entree: Pointeur description du Box .
 * Sortie: GtkWidget*, Le widget Box.
 */
GtkWidget* Box_Create(BoxDesc* Desc)
{
    GtkWidget* Widget;
    // Si le box n'est pas de type "fixed"
    if (!Desc->Fixed)
    {
        // creation du box
        Widget = gtk_box_new(Desc->Orientation, Desc->Spacing);
    }
    // Box de type fixed
    else
    {
        Widget = gtk_fixed_new();
    }
    // Afficher le box
    gtk_widget_show(Widget);
    return Widget;
}

/*
* Prototype: void Box_CreateWithXml(
            WidgetIdList* IdList[26],
            XmlElement* Element,
            WidgetContainer* Parent);
* Entree: Liste des identificateur, Element XML, Conteneur(Parent).
* Sortie: vide
* Fonction: Creation du box a partire du fichier de donnée XML.
*/
void Box_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
    BoxDesc Desc;
    BoxDesc_Init(&Desc);
    // Recuperer la liste des attributs dans "Attr"
    XmlAttribute* Attr = XML_GetAttributes(Element);
    // Tant qu'il ya des attributs
    while (Attr)
    { // Recuperer le nom de l'attribut
        const char* AttrName = XML_GetAttributeName(Attr);
        // Cas d'attribut d'orientation
        if (!strcmp(AttrName, "Orientation"))
        {
            char c;
            // Recuperer la valeur de l'attribut et initialiser la structure
            XML_GetAttributeValueFormat(Attr, "%c", &c);
            Desc.Orientation = c == 'H' ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
        }
        // Cas d'attribut d'espace
        else if (!strcmp(AttrName, "Spacing"))
        {
            // Recuperer la valeur de l'attribut et initialiser la structure
            Desc.Spacing = XML_GetAttributeValueInt(Attr);
        }
        // Cas d'attribut de type fixed
        else if (!strcmp(AttrName, "Fixed"))
        {
            // Recuperer la valeur de l'attribut et initialiser la structure
            Desc.Fixed = XML_GetAttributeValueBool(Attr);
        }
        // Passer a l'attribut suivant
        Attr = XML_GetNextAttribute(Attr);
    }
    // Creation du box
    Parent->Widget = Box_Create(&Desc);
    // Lire les fils du box
    WidgetContainer_ReadChildren(IdList, Element, Parent);
    // Recuperer la liste des fils dans "Child"
    WidgetContainer* Child = Parent->Children;
    // Tant que la liste des fils est non vide
    while (Child)
    {
        if (Child->Widget)
        {
            if (Desc.Fixed)
            {
                // Ajouter les widgets fils au parent "Box"
                gtk_container_add(GTK_CONTAINER(Parent->Widget), Child->Widget);
            }
            else
            {
                Attr = XML_FindAttribute(Child->Element, "Insert");
                gboolean AddInEnd = Attr ? !strcmp(XML_GetAttributeValue(Attr), "End") : FALSE;

                Attr = XML_FindAttribute(Child->Element, "Expand");
                gboolean Expand = Attr ? XML_GetAttributeValueBool(Attr) : FALSE;

                Attr = XML_FindAttribute(Child->Element, "Fill");
                gboolean Fill = Attr ? XML_GetAttributeValueBool(Attr) : TRUE;

                Attr = XML_FindAttribute(Child->Element, "Padding");
                int Padding = Attr ? XML_GetAttributeValueInt(Attr) : 5;

                if (AddInEnd)
                {
                    gtk_box_pack_end(GTK_BOX(Parent->Widget), Child->Widget, Expand, Fill, Padding);
                }
                else
                {
                    gtk_box_pack_start(GTK_BOX(Parent->Widget), Child->Widget, Expand, Fill, Padding);
                }
            }
            // Le cas du box de type "fixed"
            if (Desc.Fixed)
            {
                // Recuperer l'attribut dans "Attr" et tester sa valeur
                Attr = XML_FindAttribute(Child->Element, "FixedPosition");
                if (Attr)
                {
                    int X, Y;
                    // Recuperer les valeurs de l'attribut
                    XML_GetAttributeValueFormat(Attr, "%d,%d", &X, &Y);
                    // Modifier le fils
                    gtk_fixed_move(GTK_FIXED(Parent->Widget), Child->Widget, X, Y);
                }
            }
        }
        // Passer au fils suivant
        Child = Child->Next;
    }
}
