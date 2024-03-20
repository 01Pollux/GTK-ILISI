#pragma once

#include "../GtkEssentiels.h"

typedef struct GridTdData
{
    GtkWidget* Widget;
    int Width;
    int Height;

    struct GridTdData* Next;
} GridTdData;

typedef struct GridTrData
{
    GridTdData* Head;
    struct GridTrData* Next;
} GridTrData;

typedef struct
{
    GridTrData* List;
    gboolean RowHomogeneous;
    gboolean ColumnHomogeneous;
    int RowSpacing;
    int ColumnSpacing;
} GridDesc;

void GridDesc_Init(GridDesc* Desc);

GridTdData* GridTdData_Add(GridTdData* Head, GtkWidget* Widget, int Width, int Height);

GridTrData* GridTrData_Add(GridTrData* Head, GridTdData* TdData);

void GridTrData_Free(GridTrData* Data);

GtkWidget* Grid_Create(GridDesc* Desc);

void Grid_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent);
