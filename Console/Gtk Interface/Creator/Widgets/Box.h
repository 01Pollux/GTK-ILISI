#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
	gboolean Fixed;
	gint Spacing;
	GtkOrientation Orientation;
} BoxDesc;

void BoxDesc_Init(BoxDesc* Desc);

GtkWidget* Box_Create(BoxDesc* Desc);

void Box_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);