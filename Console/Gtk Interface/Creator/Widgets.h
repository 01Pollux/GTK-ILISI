#pragma once

#include "Widgets/Toolbar.h"
#include "Widgets/Window.h"
#include "Widgets/Box.h"
#include "Widgets/Button.h"
#include "Widgets/Combo.h"
#include "Widgets/Text_Input.h"
#include "Widgets/Image.h"
#include "Widgets/Label.h"
#include "Widgets/ScrolledWindow.h"
#include "Widgets/TextArea.h"

//


WidgetLayout* WidgetLayout_ReadXml(XmlDocument* Document);

void WidgetLayout_Free(WidgetLayout* Layout);

WidgetContainer* WidgetLayout_GetElementById(WidgetLayout* Layout, const char* Id);

//

void WidgetContainer_PrivateElements(WidgetIdList* IdList[26], WidgetContainer* Tree, XmlElement* Element);

//

void Widget_AddStyle(GtkWidget* Widget, const gchar* Css);