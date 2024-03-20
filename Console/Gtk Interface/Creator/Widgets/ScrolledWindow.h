#pragma once


#include "../GtkEssentiels.h"


typedef struct
{
	struct
	{
		gboolean Active;
		GtkPolicyType Visibility;

		gdouble Value;
		gdouble Lower;
		gdouble Upper;
		gdouble StepIncrement;
		gdouble PageIncrement;
		gdouble PageSize;
	} HAdjustment, VAdjustment;

	GtkCornerType Corner;
} ScrolledWindowDesc;

void ScrolledWindowDesc_Init(ScrolledWindowDesc* Desc);

GtkWidget* ScrolledWindow_Create(ScrolledWindowDesc* Desc);

void ScrolledWindow_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
