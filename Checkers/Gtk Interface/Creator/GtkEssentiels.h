#pragma once

#include <gtk/gtk.h>
#include "XmlReader.h"

typedef struct WidgetContainer
{
    struct WidgetContainer* Children;
    struct WidgetContainer* Next;
    struct WidgetContainer* Parent;

    GtkWidget* Widget;
    XmlElement* Element;
    GtkAccelGroup* AccelGroup;
} WidgetContainer;

typedef struct WidgetIdList
{
    char Id[25];
    WidgetContainer* Container;
    struct WidgetIdList* Next;
} WidgetIdList;

typedef struct
{
    WidgetIdList* IdList[26];
    WidgetContainer* WindowList;
    XmlDocument* Document;
} WidgetLayout;

WidgetContainer* WidgetIdList_GetElementById(WidgetIdList* IdList[26], const char* Id);

WidgetContainer* WidgetContainer_Read(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);

WidgetContainer* WidgetContainer_ReadPrivate(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);

void WidgetContainer_ReadChildren(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);

//

WidgetContainer* WidgetContainer_Create(GtkWidget* Widget);

WidgetContainer* WidgetContainer_CreateContainer(WidgetContainer* Head, GtkWidget* Widget);

WidgetContainer* WidgetContainer_AddContainer(WidgetContainer* Head, WidgetContainer* Queue);

//

void Malloc_Valid(void* Data, const char* NomDeFonction);

void Widget_ParseAccelerator(const char* Accelerator, int* Key, GdkModifierType* Mod);
