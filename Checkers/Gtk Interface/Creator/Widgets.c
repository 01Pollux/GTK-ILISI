#include "Widgets.h"
#include <ctype.h>

static void WidgetIdList_Add(WidgetIdList* IdList[26], WidgetContainer* Container, const char* Id);

/*
 * Prototype: void WidgetContainer_Free(WidgetContainer* Tree);
 * Entree: Pointeur arbre WidgetContainer.
 * Sortie: Vide.
 * Fonction: Liberation de l'espace memoire reservé pour les
 * widgets crée a partir du fichier XML lu apres fermeture.
 */
void WidgetContainer_Free(WidgetContainer* Tree)
{
    // Si l'arbre existe
    if (Tree)
    {
        // Liberation recursive:
        // des fils:
        WidgetContainer_Free(Tree->Children);
        // des suivants:
        WidgetContainer_Free(Tree->Next);
        // Destruction des widgets
        gtk_widget_destroy(Tree->Widget);
        // Liberation de l'espace memoire
        free(Tree);
    }
}

/*
* Prototype: WidgetContainer* WidgetContainer_Read(
    WidgetIdList* IdList[26],
    XmlElement* Element,
    WidgetContainer* Parent);
* Entree: Une liste des ID, l'element lu de l'XML, le
* parent conteneur (Widget)
* Sortie: Un conteneur widget parent finale.
*/
WidgetContainer* WidgetContainer_ReadImpl(
    WidgetIdList* IdList[26],
    XmlElement* Element,
    WidgetContainer* Parent,
    GtkAccelGroup* AccelGroup)
{
    // Creation, Initialisation et allocation de memoire a une nouvelle arbre
    WidgetContainer* Tree = (WidgetContainer*)calloc(1, sizeof(WidgetContainer));
    Malloc_Valid(Tree, "WidgetContainer_Read\n");
    // S'il n'y a pas de l'espace memoire
    if (!Tree)
    {
        printf("Erreur d'allocation\n");
        exit(-1);
    }

    // Sinon
    // Affectation
    Tree->Parent = Parent;
    Tree->Element = Element;
    Tree->AccelGroup = AccelGroup;

    // Lecture de l'element ID afin de lier les widgets de meme ID
    XmlAttribute* IdAttr = XML_FindAttribute(Element, "Id");
    // S'il existe
    if (IdAttr)
    {
        // On ajoute l'element a la liste de meme ID
        WidgetIdList_Add(IdList, Tree, XML_GetAttributeValue(IdAttr));
    }

    // Importation de nom d'element depuis le fichier
    const char* ElementName = XML_GetElementName(Element);

    /*
     * Comparaison de nom de l'element lu
     * Afin de creer le widget trouvé et l'ajouter a l'arbre
     * En lisant leurs fils...
     */
    if (!strcmp(ElementName, "Window"))
    {
        Window_CreateWithXml(IdList, Element, Tree, AccelGroup);
    }
    else if (!strcmp(ElementName, "Headerbar"))
    {
        Headerbar_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "Box"))
    {
        Box_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "Toolbar"))
    {
        MenuToolbar_CreateBarWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "Button"))
    {
        Button_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "ComboBox"))
    {
        ComboBox_CreateFromXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "TextInput"))
    {
        TextInput_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "Image"))
    {
        Image_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "Label"))
    {
        Label_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "ScrolledWindow"))
    {
        ScrolledWindow_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "TextArea"))
    {
        TextArea_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "Notebook"))
    {
        Notebook_CreateWithXml(IdList, Element, Tree);
    }
    else if (!strcmp(ElementName, "Grid"))
    {
        Grid_CreateWithXml(IdList, Element, Tree);
    }

    WidgetContainer_PrivateElements(IdList, Tree, Element);

    // Retour
    return Tree;
}
/*
* Prototype: WidgetContainer* WidgetContainer_Read(
    WidgetIdList* IdList[26],
    XmlElement* Element,
    WidgetContainer* Parent);
* Entree: Une liste des ID, l'element lu de l'XML, le
* parent conteneur (Widget)
* Sortie: Un conteneur widget parent finale.
*/
WidgetContainer* WidgetContainer_Read(
    WidgetIdList* IdList[26],
    XmlElement* Element,
    WidgetContainer* Parent)
{
    return WidgetContainer_ReadImpl(IdList, Element, Parent, NULL);
}

/*
* Prototype: void WidgetContainer_ReadChildren(WidgetIdList* IdList[26],
    XmlElement* Element,
    WidgetContainer* Parent);
* Entree: Liste des IDs, l'element XML et le parent widget
* Sortie: Vide
* Fonction: Lecture des sous widgets (Fils) dans le fichier de donnée
* et l'ajout dans la structure des données
*/
void WidgetContainer_ReadChildren(WidgetIdList* IdList[26],
                                  XmlElement* Element,
                                  WidgetContainer* Parent)
{
    // Creation et initialisation de structure des fils
    WidgetContainer *Children = NULL, *Tmp;
    // Importation des elements
    XmlElement* SubItem = XML_GetElementChildren(Element);

    // Tant qu'il y a des elements
    while (SubItem)
    {
        // Lecture avec un pointeur temporaire de l'element
        Tmp = WidgetContainer_Read(IdList, SubItem, Parent);
        // S'il existe
        if (Tmp)
        {
            // On l'ajoute a la structure des fils
            Children = WidgetContainer_AddContainer(Children, Tmp);
        }
        // On passe au suivant
        SubItem = XML_GetNextElement(SubItem);
    } // Fin tant que

    // A la fin de boucle, s'il y a au moins un fils
    if (Children)
    {
        // On ajoute la structure des fils à la structure parent
        Parent->Children = WidgetContainer_AddContainer(Parent->Children, Children);
    }
}

/*
 * Prototype:WidgetLayout* WidgetLayout_ReadXml(XmlDocument* Document);
 * Entree: Le document XML (fichier)
 * Sortie: Pointeur de type WidgetLayout
 */

WidgetLayout* WidgetLayout_ReadXml(XmlDocument* Document)
{
    // Creation, initialisation et allocation de la memoire
    WidgetLayout* Layout = (WidgetLayout*)calloc(1, sizeof(WidgetLayout));
    Malloc_Valid(Layout, "WidgetLayout_ReadXml\n");
    // Test de la memoire
    if (!Layout)
    {
        printf("Erreur d'allocation\n");
        exit(-1);
    }
    // Le document de layout pointe vers le document xml
    Layout->Document = Document;
    // On debute avec le premier element
    XmlElement* Element = XML_GetFirstElement(Document);
    // On cree une fenetre principale
    WidgetContainer* Window;

    // Tant qu'il y a des elements
    while (Element)
    {
        // On le lit, avec ses attrubut
        Window = WidgetContainer_ReadImpl(Layout->IdList, Element, NULL, gtk_accel_group_new());
        // On l'ajout a la fenetre principale
        Layout->WindowList = WidgetContainer_AddContainer(Layout->WindowList, Window);
        // On passe vers l'element suivant
        Element = XML_GetNextElement(Element);
    } // Fin tant que
    // Retour
    return Layout;
}

/*
 * Prototype: void WidgetLayout_Free(WidgetLayout* Layout);
 * Entree: Structure de type Layoute (Pointeur)
 * Sortie: Vide
 * Fonction: Liberation de memoire de tout les elements
 */
void WidgetLayout_Free(WidgetLayout* Layout)
{
    WidgetIdList *Tmp, *List;
    for (int i = 0; i < 26; i++)
    {
        List = Layout->IdList[i];
        while (List)
        {
            Tmp = List;
            List = List->Next;
            free(Tmp);
        }
    }

    WidgetContainer_Free(Layout->WindowList);
    XML_Free(Layout->Document);
    free(Layout);
}

/*
 * Prototype: WidgetContainer* WidgetLayout_GetElementById(WidgetLayout* Layout, const char* Id);
 * Entree: Un pointeur layout (La structure) et un ID a rechercher
 * Sortie: Un WidgetContainer.
 * Fonction: On cherche l'element qui porte l'id passé par argument, et on
 * le retourne.
 */
WidgetContainer* WidgetLayout_GetElementById(WidgetLayout* Layout, const char* Id)
{
    // Appel de la fonction suivante
    return WidgetIdList_GetElementById(Layout->IdList, Id);
}

/*
 * Prototype: WidgetContainer* WidgetIdList_GetElementById(WidgetIdList* IdList[26],
 * const char* Id);
 * Fonction: recherche de l'id correspondant dans la liste des ID.
 */
WidgetContainer* WidgetIdList_GetElementById(WidgetIdList* IdList[26], const char* Id)
{
    char c = tolower(*Id);
    int Index = c - 'a';
    if (Index < 0 || Index >= 26)
    {
        return NULL;
    }

    WidgetIdList* List = IdList[Index];
    while (List)
    {
        if (!strcmp(Id, List->Id))
        {
            return List->Container;
        }
        List = List->Next;
    }

    printf("[Warning]: Couldn't find widget with id '%s'\n", Id);
    return NULL;
}

/*
 * Prototype: void WidgetContainer_AddStyles(GtkWidget* Widget, XmlElement* Element);
 * Entree: le widget, et l'element XML
 * Fonction: Ajout de style a au elements lu
 */

void WidgetContainer_AddStyles(GtkWidget* Widget, XmlElement* Element)
{

    // tant qu'il y a des fils
    while (Element)
    {
        XmlAttribute* Attr = XML_FindAttribute(Element, "Text");
        if (Attr)
        {
            // On l'ajoute au widget
            Widget_AddStyle(Widget, XML_GetAttributeValue(Attr));
        } // Fin tant que
        Element = XML_GetNextElement(Element);
    }
}

/*
 * Prototype: void WidgetContainer_PrivateElements(WidgetContainer* Tree, XmlElement* Element);
 * Fonction: Lecture des elements (attributs/widgets) d'un element private
 */
void WidgetContainer_PrivateElements(WidgetIdList* IdList[26], WidgetContainer* Tree, XmlElement* Element)
{
    if (!Tree->Widget)
    {
        return;
    }

    const char* ElementName;
    Element = XML_GetElementChildren(Element);
    XmlElement* Private = NULL;

    while (Element)
    {
        ElementName = XML_GetElementName(Element);
        // Si l'element est "private"
        if (!strcmp(ElementName, "Private"))
        {
            Private = Element;
            break;
        }
        Element = XML_GetNextElement(Element);
    }

    if (!Private)
    {
        return;
    }

    XmlAttribute* Attr;
    Element = XML_GetElementChildren(Private);

    while (Element)
    {
        const char* Name = XML_GetElementName(Element);

        if (!strcmp(Name, "Style"))
        {
            if (Attr = XML_FindAttribute(Element, "Name"))
            {
                gtk_widget_set_name(Tree->Widget, XML_GetAttributeValue(Attr));
            }
            WidgetContainer_AddStyles(Tree->Widget, XML_GetElementChildren(Element));
        }
        else if (!strcmp(Name, "Size"))
        {
            int Width = -1, Height = -1;
            if (Attr = XML_FindAttribute(Element, "Width"))
            {
                Width = atoi(XML_GetAttributeValue(Attr));
            }
            if (Attr = XML_FindAttribute(Element, "Height"))
            {
                Height = atoi(XML_GetAttributeValue(Attr));
            }
            gtk_widget_set_size_request(Tree->Widget, Width, Height);
        }
        else if (!strcmp(Name, "Hidden"))
        {
            gtk_widget_hide(Tree->Widget);
        }
        else if (!strcmp(Name, "Align"))
        {
            if (Attr = XML_FindAttribute(Element, "Vertical"))
            {
                const char* AttrValue = XML_GetAttributeValue(Attr);
                if (!strcmp(AttrValue, "Start"))
                {
                    gtk_widget_set_valign(Tree->Widget, GTK_ALIGN_START);
                }
                else if (!strcmp(AttrValue, "Center"))
                {
                    gtk_widget_set_valign(Tree->Widget, GTK_ALIGN_CENTER);
                }
                else if (!strcmp(AttrValue, "End"))
                {
                    gtk_widget_set_valign(Tree->Widget, GTK_ALIGN_END);
                }
                else if (!strcmp(AttrValue, "Fill"))
                {
                    gtk_widget_set_valign(Tree->Widget, GTK_ALIGN_FILL);
                }
            }
            if (Attr = XML_FindAttribute(Element, "Horizontal"))
            {
                const char* AttrValue = XML_GetAttributeValue(Attr);
                if (!strcmp(AttrValue, "Start"))
                {
                    gtk_widget_set_halign(Tree->Widget, GTK_ALIGN_START);
                }
                else if (!strcmp(AttrValue, "Center"))
                {
                    gtk_widget_set_halign(Tree->Widget, GTK_ALIGN_CENTER);
                }
                else if (!strcmp(AttrValue, "End"))
                {
                    gtk_widget_set_halign(Tree->Widget, GTK_ALIGN_END);
                }
                else if (!strcmp(AttrValue, "Fill"))
                {
                    gtk_widget_set_halign(Tree->Widget, GTK_ALIGN_FILL);
                }
            }
        }
        else if (!strcmp(Name, "Tooltip"))
        {
            if (Attr = XML_FindAttribute(Element, "Window"))
            {
                GtkWidget* Window = WidgetIdList_GetElementById(IdList, XML_GetAttributeValue(Attr))->Widget;
                gtk_widget_set_tooltip_window(Tree->Widget, GTK_WINDOW(Window));
            }
            if (Attr = XML_FindAttribute(Element, "Text"))
            {
                gtk_widget_set_tooltip_text(Tree->Widget, XML_GetAttributeValue(Attr));
            }
        }
        Element = XML_GetNextElement(Element);
    }
}

/*
 * Prototype: static void WidgetIdList_Add(WidgetIdList* IdList[26], WidgetContainer* Container, const char* Id);
 * Entree: Liste des IDs, conteneur, ID
 * Sortie: Vide
 * Ajout d'un conteneur et son ID dans la liste des IDs
 */
static void WidgetIdList_Add(WidgetIdList* IdList[26], WidgetContainer* Container, const char* Id)
{
    char c = tolower(*Id);
    int Index = c - 'a';
    if (Index < 0 || Index >= 26)
    {
        return;
    }

    WidgetIdList* Element = (WidgetIdList*)calloc(1, sizeof(WidgetIdList));
    Malloc_Valid(Element, "WidgetIdList_Add\n");
    Element->Container = Container;
    strncpy(Element->Id, Id, 24);

    if (IdList[Index])
    {
        Element->Next = IdList[Index];
    }
    IdList[Index] = Element;
}

/*
 * Prototype: WidgetContainer* WidgetContainer_Create(GtkWidget* Widget);
 * Entree: Un GtkWidget qui fera objet d'un conteneur
 * Sortie: Le GtkWidget encapsulé dans une nouvelle structure
 */
WidgetContainer* WidgetContainer_Create(GtkWidget* Widget)
{
    // Initialisation et allocation
    WidgetContainer* Tree = (WidgetContainer*)calloc(1, sizeof(WidgetContainer));
    Malloc_Valid(Tree, "WidgetContainer_Create\n");
    // Encapsulation
    Tree->Widget = Widget;
    return Tree;
}

/*
 * Prototype: WidgetContainer* WidgetContainer_CreateContainer(WidgetContainer* Head, GtkWidget* Widget);
 * Fonction: La creation et l'ajout d'un widget a la liste des widgets
 */
WidgetContainer* WidgetContainer_CreateContainer(WidgetContainer* Head, GtkWidget* Widget)
{
    return WidgetContainer_AddContainer(Head, WidgetContainer_Create(Widget));
}

/*
 * Prototype: WidgetContainer* WidgetContainer_AddContainer(WidgetContainer* Head, WidgetContainer* Queue);
 * Fonction: Parcours de liste pour ajouter un element
 */
WidgetContainer* WidgetContainer_AddContainer(WidgetContainer* Head, WidgetContainer* Queue)
{
    // S'il n y'a pas d'elements dand Head, on retourne l'element Queue
    if (!Head)
    {
        return Queue;
    }
    // Sinon
    WidgetContainer* Tmp = Head;
    // Tant qu'il y a des elements dans head
    while (Tmp->Next)
    {
        // On passe au suivant
        Tmp = Tmp->Next;
    }
    // Ajout de queue dans la fin de la liste
    Tmp->Next = Queue;
    // Retour de la nouvelle liste
    return Head;
}

void Malloc_Valid(void* Data, const char* NomDeFonction)
{
    if (!Data)
    {
        printf("Erreur d'allocation mémoire (%s)", NomDeFonction);
        exit(-1);
    }
}

void Widget_ParseAccelerator(
    const char* Accelerator,
    int* Key,
    GdkModifierType* Mod)
{
    char Mask[31];
    char C;
    sscanf(Accelerator, "%s %c", Mask, &C);
    *Key = C;

    *Mod = 0;
    char* Tok = strtok(Mask, "+");
    while (Tok)
    {
        if (!_stricmp(Tok, "ctrl"))
        {
            *Mod |= GDK_CONTROL_MASK;
        }
        else if (!_stricmp(Tok, "shift"))
        {
            *Mod |= GDK_SHIFT_MASK;
        }
        else if (!_stricmp(Tok, "alt"))
        {
            *Mod |= GDK_MOD1_MASK;
        }
        else if (!_stricmp(Tok, "win"))
        {
            *Mod |= GDK_SUPER_MASK;
        }
        Tok = strtok(NULL, "+");
    }
}

/*
 * Prototype: void Widget_AddStyle(GtkWidget* Widget, const gchar* Css);
 * Entree: Un GtkWidget et une chaine de caractere de code CSS
 * Sortie: Vide
 * Fonction: L'ajout de style a un widget
 */
void Widget_AddStyle(GtkWidget* Widget, const gchar* Css)
{
    // Declaration d'une variable de CSS provider
    GtkCssProvider* Provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(Provider, Css, -1, NULL);

    GtkStyleContext* Context = gtk_widget_get_style_context(Widget);
    gtk_style_context_add_provider(Context, GTK_STYLE_PROVIDER(Provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    g_object_unref(Provider);
}
