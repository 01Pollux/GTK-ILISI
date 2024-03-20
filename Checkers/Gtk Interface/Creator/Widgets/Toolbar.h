#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
    const char* Name;
    const char* Icon;
    const char* Accel;
    GtkAccelGroup* Group;
} MenuItemEntry;

typedef struct MenuItemList
{
    struct MenuItemList* Children;
    struct MenuItemList* NextElement;

    GtkWidget* Widget;
} MenuItemList;

//

MenuItemList* MenuItemList_AddItem(MenuItemList* Parent, MenuItemEntry* Entry, MenuItemList* Children);

void MenuItemList_Free(MenuItemList* Items);

//

GtkWidget* MenuToolbar_CreateBar(MenuItemList* List, GtkOrientation Orientation);

void MenuToolbar_CreateBarWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
