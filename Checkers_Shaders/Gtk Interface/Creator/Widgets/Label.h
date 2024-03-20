#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
    const char* Text;
    gboolean    Wrap;
} LabelDesc;

void LabelDesc_Init(LabelDesc* Desc);

GtkWidget* Label_Create(LabelDesc* Desc);

void Label_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
