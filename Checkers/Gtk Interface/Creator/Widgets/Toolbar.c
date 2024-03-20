#include "Toolbar.h"
#include "../Widgets.h"

/*
 * Adding an item (child) to a Menu List (parent).
 */
static MenuItemList* MenuItemList_AddItemImpl(MenuItemList* Parent, MenuItemEntry* Entry, MenuItemList* Children, MenuItemList* NewElement[1])
{
    MenuItemList* Item = (MenuItemList*)calloc(1, sizeof(MenuItemList));
    Malloc_Valid(Item, "MenuItemList_AddItemImpl\n");

    if (Entry->Name)
    {
        if (Entry->Icon)
        {
            gboolean IsCustomIcon = strstr(Entry->Icon, ".") != NULL;

            GtkWidget* Icon;
            if (IsCustomIcon)
            {
                GdkPixbuf* Buf = gdk_pixbuf_new_from_file_at_scale(Entry->Icon, 16, 16, TRUE, NULL);
                Icon = gtk_image_new_from_pixbuf(Buf);
            }
            else
            {
                Icon = gtk_image_new_from_icon_name(Entry->Icon, GTK_ICON_SIZE_MENU);
            }

            GtkWidget* Label = !Entry->Accel ? gtk_label_new(Entry->Name) : gtk_accel_label_new(Entry->Name);
            GtkWidget* Box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

            Item->Widget = gtk_menu_item_new();

            if (Entry->Accel)
            {
                gtk_label_set_use_underline(GTK_LABEL(Label), TRUE);
                gtk_label_set_xalign(GTK_LABEL(Label), 0.f);
                gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(Label), Item->Widget);
            }

            gtk_container_add(GTK_CONTAINER(Box), Icon);
            gtk_box_pack_end(GTK_BOX(Box), Label, TRUE, TRUE, 0);
            gtk_container_add(GTK_CONTAINER(Item->Widget), Box);

            gtk_widget_show_all(Box);
        }
        else
        {
            Item->Widget = gtk_menu_item_new_with_label(Entry->Name);
        }

        if (Entry->Accel)
        {
            int Key;
            GdkModifierType Mask;
            Widget_ParseAccelerator(Entry->Accel, &Key, &Mask);

            gtk_widget_add_accelerator(
                Item->Widget,
                "activate",
                Entry->Group,
                Key, Mask, GTK_ACCEL_VISIBLE);
        }
    }
    else
    {
        Item->Widget = gtk_separator_menu_item_new();
    }

    Item->Children = Children;
    NewElement[0] = Item;

    if (!Parent)
    {
        return Item;
    }

    MenuItemList* Tmp = Parent;
    while (Tmp->NextElement)
    {
        Tmp = Tmp->NextElement;
    }
    Tmp->NextElement = Item;
    return Parent;
}

MenuItemList* MenuItemList_AddItem(MenuItemList* Parent, MenuItemEntry* Entry, MenuItemList* Children)
{
    MenuItemList* Empty[1];
    return MenuItemList_AddItemImpl(Parent, Entry, Children, Empty);
}

/*
 * Freeing a Menu List.
 */
void MenuItemList_Free(MenuItemList* Items)
{
    MenuItemList* Tmp;
    while (Items)
    {
        MenuItemList_Free(Items->Children);
        Tmp = Items;
        Items = Items->NextElement;
        free(Tmp);
    }
}

/*
 * Creating submenus, implementing the list.
 */

static void MenuToolbar_CreateBar_Impl(GtkWidget* Widget, MenuItemList* List)
{
    if (List)
    {

        GtkWidget* SubMenu = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(Widget), SubMenu);

        while (List)
        {
            MenuToolbar_CreateBar_Impl(List->Widget, List->Children);
            gtk_menu_shell_append(GTK_MENU_SHELL(SubMenu), List->Widget);
            List = List->NextElement;
        }
    }
}

/*
 * Creating a Menu (Menu tool bar).
 */
GtkWidget* MenuToolbar_CreateBar(MenuItemList* List, GtkOrientation Orientation)
{
    GtkWidget* MenuBar = gtk_menu_bar_new();
    gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(MenuBar), Orientation);

    while (List)
    {
        MenuToolbar_CreateBar_Impl(List->Widget, List->Children);
        gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar), List->Widget);
        List = List->NextElement;
    }

    gtk_widget_show_all(MenuBar);
    return MenuBar;
}

//

MenuItemList* MenuToolbar_CreateBarWithXmlImpl(
    WidgetIdList* IdList[26],
    XmlElement* Element,
    WidgetContainer* Parent,
    MenuItemList* ParentItems)
{
    MenuItemList* List = NULL;
    MenuItemList* Children;
    MenuItemList* NewElement[1];

    WidgetContainer* Cont;
    XmlAttribute* Attr;

    MenuItemEntry Entry;

    while (Element)
    {
        if (!strcmp(XML_GetElementName(Element), "ToolbarItem"))
        {
            Cont = WidgetContainer_Read(IdList, Element, Parent);
            Parent->Children = WidgetContainer_AddContainer(Parent->Children, Cont);

            Children = MenuToolbar_CreateBarWithXmlImpl(IdList, XML_GetElementChildren(Element), Cont, ParentItems);

            Attr = XML_FindAttribute(Element, "Name");
            Entry.Name = Attr ? XML_GetAttributeValue(Attr) : NULL;

            Attr = XML_FindAttribute(Element, "Icon");
            Entry.Icon = Attr ? XML_GetAttributeValue(Attr) : NULL;

            Attr = XML_FindAttribute(Element, "Accel");
            Entry.Accel = Attr ? XML_GetAttributeValue(Attr) : NULL;
            if (Entry.Accel)
            {
                WidgetContainer* Window = Parent;
                while (Window->Parent)
                {
                    Window = Window->Parent;
                }
                Entry.Group = Window->AccelGroup;
            }

            List = MenuItemList_AddItemImpl(List, &Entry, Children, NewElement);
            Cont->Widget = NewElement[0]->Widget;

            WidgetContainer_PrivateElements(IdList, Cont, Element);
        }
        Element = XML_GetNextElement(Element);
    }
    return List;
}

void MenuToolbar_CreateBarWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
    GtkPackDirection Direction = GTK_PACK_DIRECTION_LTR;
    XmlAttribute* Attr = XML_FindAttribute(Element, "Direction");
    if (Attr)
    {
        const char* AttrName = XML_GetAttributeValue(Attr);
        if (!strcmp(AttrName, "RTL"))
        {
            Direction = GTK_PACK_DIRECTION_RTL;
        }
        else if (!strcmp(AttrName, "TTB"))
        {
            Direction = GTK_PACK_DIRECTION_TTB;
        }
        else if (!strcmp(AttrName, "BTT"))
        {
            Direction = GTK_PACK_DIRECTION_BTT;
        }
    }

    MenuItemList* Items = MenuToolbar_CreateBarWithXmlImpl(IdList, XML_GetElementChildren(Element), Parent, NULL);
    GtkWidget* Widget = MenuToolbar_CreateBar(Items, Direction);
    MenuItemList_Free(Items);
    Parent->Widget = Widget;
}
