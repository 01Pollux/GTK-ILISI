#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
	gint x;
	gint y;
} Position;

typedef struct
{
	gint Height;
	gint Width;
	gdouble Opacity;
	const gchar* Title;
	gboolean Deletable;
	gboolean Iconified;
	gboolean Resizable;
	gboolean Fullscreen;
	gboolean Decorations;
	gboolean Maximize;
	const gchar* Icon;
} WindowDesc;

void WindowDesc_Init(WindowDesc* Desc);

GtkWidget* Window_Create(WindowDesc* Desc);

GtkWidget* Window_CreatePopup(WindowDesc* Desc);

GtkWidget* Window_ChangeTitle(GtkWindow* Window, char* title);

void Window_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);