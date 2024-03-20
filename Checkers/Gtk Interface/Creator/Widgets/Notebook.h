#pragma once

#include "../GtkEssentiels.h"

typedef struct
{
    const char*     TabName;
    gboolean        Detach;
    gboolean        Reorder;
    const char*     MenuName;
    gboolean        Scrollalble;
    gboolean        ShowTabs;
    gboolean        ShowBorder;
    gboolean        Expand;
    gboolean        Fill;
    guint           Padding;
    GtkPositionType Position;
} NotebookEntryDesc;

typedef struct NotebookEntry
{
    struct NotebookEntry* Next;
    struct NotebookEntry* Children;
    GtkWidget*            Page;
    GtkWidget*            Label;
    NotebookEntryDesc     Desc;
} NotebookEntry;

void NotebookEntryDesc_Init(NotebookEntryDesc* Desc);

NotebookEntry* NotebookEntry_Create(GtkWidget* Page, NotebookEntryDesc* Desc);

NotebookEntry* NotebookEntry_Append(NotebookEntry* Container, NotebookEntry* Tab);

GtkWidget* Notebook_Create(NotebookEntry* List, NotebookEntryDesc* Desc);

void Notebook_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
