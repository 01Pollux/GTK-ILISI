#include "Notebook.h"

NotebookEntry* NotebookEntry_Create(GtkWidget* Page, NotebookEntryDesc* Desc)
{
    NotebookEntry* Entry = (NotebookEntry*)calloc(1, sizeof(NotebookEntry));
    Malloc_Valid(Entry, "NotebookEntry_Create");

    Entry->Page  = Page;
    Entry->Label = gtk_label_new(Desc->TabName);
    gtk_widget_show(Entry->Label);
    Entry->Desc = *Desc;

    return Entry;
}

void NotebookEntryDesc_Init(NotebookEntryDesc* Desc)
{
    Desc->Fill        = TRUE;
    Desc->Expand      = TRUE;
    Desc->MenuName    = NULL;
    Desc->TabName     = "Tab";
    Desc->Reorder     = TRUE;
    Desc->Detach      = TRUE;
    Desc->ShowBorder  = TRUE;
    Desc->ShowTabs    = TRUE;
    Desc->Padding     = 0;
    Desc->Scrollalble = TRUE;
    Desc->Position    = GTK_POS_TOP;
}

NotebookEntry* NotebookEntry_Append(NotebookEntry* Container, NotebookEntry* Tab)
{
    if (!Container)
    {
        Container = Tab;
    }
    else
    {
        NotebookEntry* Queue = Container;
        while (Queue->Next)
        {
            Queue = Queue->Next;
        }
        Queue->Next = Tab;
    }
    return Container;
}

//

static GtkWidget* Notebook_Create_Impl(NotebookEntry* List, NotebookEntryDesc* Desc)
{
    GtkWidget*   Notebook = gtk_notebook_new();
    GtkNotebook* Nb       = GTK_NOTEBOOK(Notebook);

    gtk_notebook_set_scrollable(Nb, Desc->Scrollalble);
    gtk_notebook_set_show_tabs(Nb, Desc->ShowTabs);
    gtk_notebook_set_show_border(Nb, Desc->ShowBorder);
    gtk_notebook_set_tab_pos(Nb, Desc->Position);

    while (List)
    {
        gtk_notebook_append_page(GTK_NOTEBOOK(Notebook), List->Page, List->Label);

        gtk_notebook_set_tab_detachable(Nb, List->Page, List->Desc.Detach);
        gtk_notebook_set_tab_reorderable(Nb, List->Page, List->Desc.Reorder);

        if (List->Desc.MenuName)
        {
            gtk_notebook_set_menu_label_text(Nb, List->Page, List->Desc.MenuName);
            gtk_notebook_popup_enable(Nb);
        }

        if (List->Children)
        {
            GtkWidget* Child = Notebook_Create_Impl(List->Children, &List->Desc);
            gtk_box_pack_start(GTK_BOX(List->Page), Child, List->Desc.Expand, List->Desc.Fill, List->Desc.Padding);
        }
        List = List->Next;
    }
    gtk_widget_show(Notebook);
    return Notebook;
}

GtkWidget* Notebook_Create(NotebookEntry* List, NotebookEntryDesc* Desc)
{
    return Notebook_Create_Impl(List, Desc);
}

//

static void NotebookEntryDesc_FillXml(NotebookEntryDesc* Desc, XmlElement* Info)
{
    XmlAttribute* Attr = XML_GetAttributes(Info);
    while (Attr)
    {
        const char* AttrName = XML_GetAttributeName(Attr);
        if (!strcmp(AttrName, "TabName"))
        {
            Desc->TabName = XML_GetAttributeValue(Attr);
        }
        else if (!strcmp(AttrName, "Detach"))
        {
            Desc->Detach = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "Reorder"))
        {
            Desc->Reorder = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "MenuName"))
        {
            Desc->MenuName = XML_GetAttributeValue(Attr);
        }
        else if (!strcmp(AttrName, "Scrollalble"))
        {
            Desc->Scrollalble = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "ShowTabs"))
        {
            Desc->ShowTabs = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "ShowBorder"))
        {
            Desc->ShowBorder = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "Expand"))
        {
            Desc->Expand = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "Fill"))
        {
            Desc->Fill = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "Padding"))
        {
            Desc->Padding = XML_GetAttributeValueInt(Attr);
        }
        else if (!strcmp(AttrName, "Position"))
        {
            const char* AttrValue = XML_GetAttributeValue(Attr);
            if (!strcmp(AttrValue, "TOP"))
            {
                Desc->Position = GTK_POS_TOP;
            }
            else if (!strcmp(AttrValue, "BOTTOM"))
            {
                Desc->Position = GTK_POS_BOTTOM;
            }
            else if (!strcmp(AttrValue, "LEFT"))
            {
                Desc->Position = GTK_POS_LEFT;
            }
            else if (!strcmp(AttrValue, "RIGHT"))
            {
                Desc->Position = GTK_POS_RIGHT;
            }
        }

        Attr = XML_GetNextAttribute(Attr);
    }
}

void WidgetContainer_ReadChildrenOrNotebook(
    WidgetIdList*    IdList[26],
    XmlElement*      Element,
    WidgetContainer* Parent)
{
    // Creation et initialisation de structure des fils
    WidgetContainer *Children = NULL, *Tmp;
    // Importation des elements
    XmlElement* SubItem = XML_GetElementChildren(Element);

    // Tant qu'il y a des elements
    while (SubItem)
    {
        if (!strstr(XML_GetElementName(SubItem), "Notebook"))
        {
            // Lecture avec un pointeur temporaire de l'element
            Tmp = WidgetContainer_Read(IdList, SubItem, Parent);
            // S'il existe
            if (Tmp)
            {
                // On l'ajoute a la structure des fils
                Children = WidgetContainer_AddContainer(Children, Tmp);
            }
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

static NotebookEntry* Notebook_CreateWithXmlImpl(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent, gboolean Recursive)
{
    NotebookEntryDesc Desc;
    NotebookEntryDesc_Init(&Desc);

    XmlElement* NotebookMap = XML_FindElement(XML_GetElementChildren(Element), "NotebookMap");
    if (!NotebookMap)
    {
        printf("Missing NotebookMap element\n");
        return NULL;
    }
    NotebookMap = XML_GetElementChildren(NotebookMap);

    NotebookEntry* EntryList = NULL;

    if (!Recursive)
    {
        WidgetContainer_ReadChildrenOrNotebook(IdList, Element, Parent);
    }
    WidgetContainer* Child = Parent->Children;

    while (Child)
    {
        const char* ElmName = XML_GetElementName(Child->Element);
        if (!strcmp(ElmName, "NotebookMap"))
        {
            Child = Child->Next;
            continue;
        }

        NotebookEntryDesc_Init(&Desc);

        XmlAttribute* NbTag = XML_FindAttribute(Child->Element, "NotebookTag");
        if (NbTag)
        {
            const char* Tag  = XML_GetAttributeValue(NbTag);
            XmlElement* Info = XML_FindElement(NotebookMap, Tag);
            if (Info)
            {
                NotebookEntryDesc_FillXml(&Desc, Info);
            }
        }

        NotebookEntry* Entry = NotebookEntry_Create(Child->Widget, &Desc);
        EntryList            = NotebookEntry_Append(EntryList, Entry);

        Child = Child->Next;
    }

    return EntryList;
}

void Notebook_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
    NotebookEntryDesc Desc;
    NotebookEntryDesc_Init(&Desc);
    NotebookEntryDesc_FillXml(&Desc, Element);

    NotebookEntry* Entries = Notebook_CreateWithXmlImpl(IdList, Element, Parent, FALSE);
    Parent->Widget         = Notebook_Create(Entries, &Desc);
}
