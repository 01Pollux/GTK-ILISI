#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
    const char* Title;
    // https://docs.gtk.org/gtk3/method.HeaderBar.set_decoration_layout.html
    // Recognized button names are minimize, maximize, close, icon (the window icon) and menu (a menu button for the fallback app menu).
    // Layout == NULL, no decoration
    // The format of the string is button names, separated by commas.
    // A colon separates the buttons that should appear on the left from those on the right.
    // recognized button names are minimize, maximize, close, icon (the window icon) and menu
    //(a menu button for the fallback app menu).

    // menu:minimize,maximize,close
    const char* Layout;
} HeaderbarDesc;

void HeaderbarDesc_Init(HeaderbarDesc* Desc);

GtkWidget* Headerbar_Create(HeaderbarDesc* Desc);

void Headerbar_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
