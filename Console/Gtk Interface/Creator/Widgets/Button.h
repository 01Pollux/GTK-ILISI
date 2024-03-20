#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
	GtkWidget* Link;
	const gchar* Label;
} RadioButtonDesc;

void RadioButtonDesc_Init(RadioButtonDesc* Desc);

GtkWidget* Button_Create();

GtkWidget* Button_Label_Create(const gchar* Label);

GtkWidget* Button_Image_Create(const gchar* Path);

GtkWidget* Button_Color_Create(const GdkRGBA* Color);

GtkWidget* Button_Radio_Create(RadioButtonDesc* Desc);

void Button_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);