#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
	const char* Name;
	const char* Icon;
} MenuItemEntry;

typedef struct MenuItemList
{
	struct MenuItemList* Childrens;
	struct MenuItemList* NextElement;

	GtkWidget* Widget;
} MenuItemList;

//

MenuItemList* MenuItemList_AddItem(MenuItemList* Parent, MenuItemEntry Entry, MenuItemList* Childrens);

void MenuItemList_Free(MenuItemList* Items);

//

GtkWidget* MenuToolbar_CreateBar(MenuItemList* List, GtkOrientation Orientation);

void MenuToolbar_CreateBarWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);