#include "Window.h"

/*
 * Prototype: void WindowDesc_Init(WindowDesc* Window);
 * Entree: Pointeur description de fenetre.
 * Sortie: Modification des valeurs de la structure WindowDesc.
 */

void WindowDesc_Init(WindowDesc* Desc)
{
    // Longeur
    Desc->Height = 720;
    // Largeur
    Desc->Width = 1280;
    // Opacité
    Desc->Opacity = 1.0;
    // Titre
    Desc->Title = "The Title";
    // Supprimable si vrai
    Desc->Deletable = TRUE;
    // Plein ecran si vrai
    Desc->Fullscreen = FALSE;
    // Iconifié si vrai
    Desc->Iconified = FALSE;
    // Decoré si vrai (bouttons haut-droit)
    Desc->Decorations = TRUE;
    // Redimensonnable si vrai
    Desc->Resizable = TRUE;
    // Maximisé si vrai
    Desc->Maximize = FALSE;
    // Pointeur sur le fichier d'icon
    Desc->Icon = NULL;
}

/*
 * Prototype: GtkWidget* Window_Create(WindowDesc* Desc);
 * Entree: Pointeur sur structure de description.
 * Sortie: GtkWidget*, la fenetre elle même.
 */

GtkWidget* Window_Create(WindowDesc* Desc)
{
    // Creation d'une fenetre de type GTK_WINDOW_TOPLEVEL (ordinaire)
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /*
     * Importation de la description a partir de la structure
     * passé par argument (Desc).
     * Utilisation des fonctions GTK.
     */
    gtk_window_set_default_size(GTK_WINDOW(window), Desc->Width, Desc->Height);
    gtk_window_set_title(GTK_WINDOW(window), Desc->Title);
    gtk_widget_set_opacity(window, Desc->Opacity);
    gtk_window_set_decorated(GTK_WINDOW(window), Desc->Decorations);

    if (Desc->Fullscreen)
        gtk_window_fullscreen(GTK_WINDOW(window));

    gtk_window_set_deletable(GTK_WINDOW(window), Desc->Deletable);
    gtk_window_set_resizable(GTK_WINDOW(window), Desc->Resizable);

    if (Desc->Iconified)
        gtk_window_iconify(GTK_WINDOW(window));
    if (Desc->Maximize)
        gtk_window_maximize(GTK_WINDOW(window));
    if (Desc->Icon)
        gtk_window_set_icon_from_file(GTK_WINDOW(window), Desc->Icon, NULL);

    // Affichage de la fenetre
    gtk_widget_show(window);

    // Retour
    return window;
}

/*
 * Prototype: GtkWidget* Window_CreatePopup(WindowDesc* Desc);
 * Entree: Pointeur sur structure de description.
 * Sortie: GtkWidget*, la fenetre elle même de type popup.
 */
GtkWidget* Window_CreatePopup(WindowDesc* Desc)
{
    // Creation de la fenetre avec la fonction Window_Create();
    GtkWidget* Window = Window_Create(Desc);

    // Mettre la fenetre en type POPUP
    gtk_window_set_type_hint(GTK_WINDOW(Window), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
    // Desactivation de la barre de titre superieure
    gtk_window_set_decorated(GTK_WINDOW(Window), FALSE);

    // Creation d'un widget de type box (Conteneur)
    GtkWidget* Box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Cretion d'une barre d'en-tete
    GtkWidget* Header_Bar = gtk_header_bar_new();
    // Importation de quelques attributs
    // Bouton de fermeture
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(Header_Bar), TRUE);
    // Titre
    gtk_header_bar_set_title(GTK_HEADER_BAR(Header_Bar), Desc->Title);
    // Taille de la barre
    gtk_widget_set_size_request(Header_Bar, 10, 10);
    // Taille de la fenetre
    gtk_window_set_default_size(GTK_WINDOW(Window), 400, 400);

    // Taille des bordures de fenetre
    gtk_container_set_border_width(GTK_CONTAINER(Window), 10);

    // Ajout de la barre au box
    gtk_container_add(GTK_CONTAINER(Box), Header_Bar);
    // Ajout de box a la fenetre
    gtk_container_add(GTK_CONTAINER(Window), Box);

    // Position centré
    gtk_window_set_position(GTK_WINDOW(Window), GTK_WIN_POS_CENTER_ALWAYS);

    // Affichage de tout les widgets
    gtk_widget_show_all(Window);

    // retour
    return Window;
}

/*
 * Prototype: GtkWidget* Window_ChangeTitle(GtkWindow* Window, char* title);
 * Entree: La fenetre (pointeur), le titre (chaine de caracteres).
 * Sortie: Widget pointeur, la fenetre avec le titre changé.
 */
GtkWidget* Window_ChangeTitle(GtkWindow* Window, char* title)
{
    // Changement de titre.
    gtk_window_set_title(Window, title);
    // Retour
    return GTK_WIDGET(Window);
}

/*
* Prototype: void Window_CreateWithXml(
    WidgetIdList* IdList[26],
    XmlElement* Element,
    WidgetContainer* Parent);
* Entree: Liste des identificateur, Element XML, Conteneur(Parent).
* Sortie: vide
* Fonction: Creation de la fenetre avec le fichier de donnée XML.
*/
void Window_CreateWithXml(
    WidgetIdList* IdList[26],
    XmlElement* Element,
    WidgetContainer* Parent,
    GtkAccelGroup* AccelGroup)
{
    // Creation et initialisation de la structure Desc (description).
    WindowDesc Desc;
    WindowDesc_Init(&Desc);

    // Importation de l'attribut depuis le fichier
    XmlAttribute* Attr = XML_GetAttributes(Element);
    /*
     * Tant qu'il y a des attribut, on l'importe et compare le nom
     * Pour chaque attribut trouvé il y a une action correspondante.
     */
    while (Attr)
    {
        // Taille de fenetre
        if (!strcmp(XML_GetAttributeName(Attr), "Size"))
            XML_GetAttributeValueFormat(Attr, "%dx%d", &Desc.Width, &Desc.Height);

        // Opacité de fenetre
        else if (!strcmp(XML_GetAttributeName(Attr), "Opacity"))
            Desc.Opacity = XML_GetAttributeValueFloat(Attr);

        // Titre de fenetre
        else if (!strcmp(XML_GetAttributeName(Attr), "Title"))
            Desc.Title = XML_GetAttributeValue(Attr);

        // Si la fenetre peut etre detruite
        else if (!strcmp(XML_GetAttributeName(Attr), "Deletable"))
            Desc.Deletable = XML_GetAttributeValueBool(Attr);

        // Si la fenetre est iconifié
        else if (!strcmp(XML_GetAttributeName(Attr), "Iconified"))
            Desc.Iconified = XML_GetAttributeValueBool(Attr);

        // Si on peut redimensionner la fenetre
        else if (!strcmp(XML_GetAttributeName(Attr), "Resizable"))
            Desc.Resizable = XML_GetAttributeValueBool(Attr);

        // Si la fenetre est en plein ecran
        else if (!strcmp(XML_GetAttributeName(Attr), "Fullscreen"))
            Desc.Fullscreen = XML_GetAttributeValueBool(Attr);

        // Si la fenetre a des decorations
        else if (!strcmp(XML_GetAttributeName(Attr), "Decorations"))
            Desc.Decorations = XML_GetAttributeValueBool(Attr);

        // Fenetre maximisé
        else if (!strcmp(XML_GetAttributeName(Attr), "Maximize"))
            Desc.Maximize = XML_GetAttributeValueBool(Attr);

        // Le fichier d'icone de la fichier
        else if (!strcmp(XML_GetAttributeName(Attr), "Icon"))
            Desc.Icon = XML_GetAttributeValue(Attr);

        Attr = XML_GetNextAttribute(Attr);
    } // Fin tant que

    // Le parent est la fenetre elle meme.
    Parent->Widget = Window_Create(&Desc);

    gtk_window_add_accel_group(GTK_WINDOW(Parent->Widget), AccelGroup);

    // Lecture de tout les balise sous (en hierarchie) la balise de la fenetre
    WidgetContainer_ReadChildren(IdList, Element, Parent);

    WidgetContainer* Child = Parent->Children;
    while (Child)
    {
        if (Child->Widget)
        {
            if (strcmp(XML_GetElementName(Child->Element), "Headerbar"))
            {
                // Ajout des fils au conteneur (fenetre).
                gtk_container_add(GTK_CONTAINER(Parent->Widget), Child->Widget);
            }
            else
            {
                // Ajout de la barre d'en-tete a la fenetre.
                gtk_window_set_titlebar(GTK_WINDOW(Parent->Widget), Child->Widget);
            }
        }
        Child = Child->Next;
    }
}
