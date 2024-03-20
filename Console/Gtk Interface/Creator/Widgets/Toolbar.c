#include "Toolbar.h"
#include "../Widgets.h"

/*
* Adding an item (child) to a Menu List (parent).
*/
static MenuItemList* MenuItemList_AddItemImpl(MenuItemList* Parent, MenuItemEntry Entry, MenuItemList* Childrens, MenuItemList* NewElement[1])
{
	MenuItemList* Item = (MenuItemList*)calloc(1, sizeof(MenuItemList));
	if (!Item)
	{
		printf("Erreur d'allocation");
		exit(-1);
	}

	if (Entry.Name)
	{
		if (Entry.Icon)
		{
			gboolean IsCustomIcon = strstr(Entry.Icon, ".") != NULL;

			GtkWidget* Icon;
			if (IsCustomIcon)
			{
				GdkPixbuf* Buf = gdk_pixbuf_new_from_file_at_scale(Entry.Icon, 16, 16, TRUE, NULL);
				Icon = gtk_image_new_from_pixbuf(Buf);
			}
			else
			{
				Icon = gtk_image_new_from_icon_name(Entry.Icon, GTK_ICON_SIZE_MENU);
			}

			GtkWidget* Label = gtk_label_new(Entry.Name);
			GtkWidget* Box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

			Item->Widget = gtk_menu_item_new();

			gtk_widget_set_halign(Box, GTK_ALIGN_START);
			gtk_container_add(GTK_CONTAINER(Box), Icon);
			gtk_container_add(GTK_CONTAINER(Box), Label);
			gtk_container_add(GTK_CONTAINER(Item->Widget), Box);
		}
		else
		{
			Item->Widget = gtk_menu_item_new_with_label(Entry.Name);
		}
	}
	else
	{
		Item->Widget = gtk_separator_menu_item_new();
	}

	Item->Childrens = Childrens;
	NewElement[0] = Item;

	if (!Parent)
	{
		return Item;
	}
	else
	{
		MenuItemList* Tmp = Parent;
		while (Tmp->NextElement)
		{
			Tmp = Tmp->NextElement;
		}
		Tmp->NextElement = Item;
		return Parent;
	}
}

MenuItemList* MenuItemList_AddItem(MenuItemList* Parent, MenuItemEntry Entry, MenuItemList* Childrens)
{
	MenuItemList* Empty[1];
	return MenuItemList_AddItemImpl(Parent, Entry, Childrens, Empty);
}


/*
* Freeing a Menu List.
*/
void MenuItemList_Free(MenuItemList* Items)
{
	MenuItemList* Tmp;
	while (Items)
	{
		MenuItemList_Free(Items->Childrens);
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
			MenuToolbar_CreateBar_Impl(List->Widget, List->Childrens);
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
		MenuToolbar_CreateBar_Impl(List->Widget, List->Childrens);
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
	MenuItemList* Childrens;
	MenuItemList* NewElement[1];

	WidgetContainer* Cont;
	XmlAttribute* Attr;

	MenuItemEntry Entry;

	while (Element)
	{
		if (!strcmp(XML_GetElementName(Element), "ToolbarItem"))
		{
			Cont = WidgetContainer_Read(IdList, Element, Parent);
			Parent->Childrens = WidgetContainer_AddContainer(Parent->Childrens, Cont);

			Childrens = MenuToolbar_CreateBarWithXmlImpl(IdList, XML_GetElementChildrens(Element), Cont, ParentItems);

			Attr = XML_FindAttribute(Element, "Name");
			Entry.Name = Attr ? XML_GetAttributeValue(Attr) : NULL;

			Attr = XML_FindAttribute(Element, "Icon");
			Entry.Icon = Attr ? XML_GetAttributeValue(Attr) : NULL;

			List = MenuItemList_AddItemImpl(List, Entry, Childrens, NewElement);
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

	MenuItemList* Items = MenuToolbar_CreateBarWithXmlImpl(IdList, XML_GetElementChildrens(Element), Parent, NULL);
	GtkWidget* Widget = MenuToolbar_CreateBar(Items, Direction);
	MenuItemList_Free(Items);
	Parent->Widget = Widget;
}