#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
    gint         MaxChars;
    const gchar* StartText;
    const gchar* HintText;
    gboolean     Visible;
} TextInputDesc;

void TextInputDesc_Init(TextInputDesc* Desc);

GtkWidget* TextInput_Create(TextInputDesc* Desc);

void TextInput_Add_Icon(GtkWidget* Widget, const gchar* Path);

void TextInput_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
