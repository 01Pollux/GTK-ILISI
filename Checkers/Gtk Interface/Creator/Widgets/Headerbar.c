#include "Headerbar.h"

void HeaderbarDesc_Init(HeaderbarDesc* Desc)
{
    Desc->Title  = "Title";
    Desc->Layout = NULL;
}

GtkWidget* Headerbar_Create(HeaderbarDesc* Desc)
{
    GtkWidget* Headerbar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(Headerbar), Desc->Title);
    if (Desc->Layout)
    {
        gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(Headerbar), TRUE);
        gtk_header_bar_set_decoration_layout(GTK_HEADER_BAR(Headerbar), Desc->Layout);
    }
    gtk_widget_show(Headerbar);
    return Headerbar;
}

void Headerbar_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
    HeaderbarDesc Desc;
    HeaderbarDesc_Init(&Desc);

    XmlAttribute* Attr = XML_GetAttributes(Element);
    const char*   AttrName;

    while (Attr)
    {
        AttrName = XML_GetAttributeName(Attr);
        if (!strcmp(AttrName, "Title"))
        {
            Desc.Title = XML_GetAttributeValue(Attr);
        }
        else if (!strcmp(AttrName, "Layout"))
        {
            Desc.Layout = XML_GetAttributeValue(Attr);
        }
        Attr = XML_GetNextAttribute(Attr);
    }

    Parent->Widget = Headerbar_Create(&Desc);
    // Lire les fils du headerbar
    WidgetContainer_ReadChildren(IdList, Element, Parent);
    // Recuperer la liste des fils dans "Child"
    WidgetContainer* Child = Parent->Children;
    // Tant que la liste des fils est non vide
    while (Child)
    {
        if (Child->Widget)
        {
            Attr = XML_FindAttribute(Child->Element, "Pack");
            if (!Attr)
            {
                // Ajouter les widgets fils au parent "Box"
                gtk_container_add(GTK_CONTAINER(Parent->Widget), Child->Widget);
            }
            else
            {
                const char* Pack = XML_GetAttributeValue(Attr);
                if (!strcmp(Pack, "Start"))
                {
                    gtk_header_bar_pack_start(GTK_HEADER_BAR(Parent->Widget), Child->Widget);
                }
                else if (!strcmp(Pack, "End"))
                {
                    gtk_header_bar_pack_end(GTK_HEADER_BAR(Parent->Widget), Child->Widget);
                }
            }
        }
        // Passer au fils suivant
        Child = Child->Next;
    }
}
