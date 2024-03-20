#pragma once

#include "../GtkEssentiels.h"

typedef struct ComboBoxItem
{
	const gchar* Label;
	struct ComboBoxItem* Next;
} ComboBoxItem;

ComboBoxItem* ComboBoxItem_Add(ComboBoxItem* Head, const gchar* Label);

void ComboBoxItem_Free(ComboBoxItem* Head);

//

GtkWidget* ComboBox_Create(const ComboBoxItem* Items);

void ComboBox_CreateFromXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);