#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
	const char* Path;
	gboolean UseIcon;
	int Width;
	int Height;
	GtkIconSize Type;
} ImageDesc;

void ImageDesc_Init(ImageDesc* Desc);

GtkWidget* Image_Create(ImageDesc* Desc);

void Image_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
