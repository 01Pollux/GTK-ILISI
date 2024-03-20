#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
    const char* Text;
    gboolean    Editable;
    GtkWrapMode Wrap;
} TextAreaDesc;

void TextAreaDesc_Init(TextAreaDesc* Desc);

GtkWidget* TextArea_Create(TextAreaDesc* Desc);

void TextArea_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
