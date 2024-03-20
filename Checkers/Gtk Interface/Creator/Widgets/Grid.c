#include "Grid.h"

void GridDesc_Init(GridDesc* Desc)
{
    Desc->List = NULL;
    Desc->RowHomogeneous = TRUE;
    Desc->ColumnHomogeneous = TRUE;
    Desc->RowSpacing = 0;
    Desc->ColumnSpacing = 0;
}

GridTdData* GridTdData_Add(GridTdData* Head, GtkWidget* Widget, int Width, int Height)
{
    GridTdData* Tail = (GridTdData*)calloc(1, sizeof(GridTdData));
    Tail->Widget = Widget;
    Tail->Width = Width;
    Tail->Height = Height;
    if (!Head)
    {
        return Tail;
    }
    GridTdData* Tmp = Head;
    while (Tmp->Next)
    {
        Tmp = Tmp->Next;
    }
    Tmp->Next = Tail;
    return Head;
}

GridTrData* GridTrData_Add(GridTrData* Head, GridTdData* TdData)
{
    GridTrData* Tail = (GridTrData*)calloc(1, sizeof(GridTrData));
    Tail->Head = TdData;
    if (!Head)
    {
        return Tail;
    }
    GridTrData* Tmp = Head;
    while (Tmp->Next)
    {
        Tmp = Tmp->Next;
    }
    Tmp->Next = Tail;
    return Head;
}

void GridTrData_Free(GridTrData* Data)
{
    GridTrData* Tr;
    GridTdData* Td;
    while (Data)
    {
        while (Data->Head)
        {
            Td = Data->Head;
            Data->Head = Td->Next;
            free(Td);
        }
        Tr = Data;
        Data = Data->Next;
        free(Tr);
    }
}

GtkWidget* Grid_Create(GridDesc* Desc)
{
    GtkGrid* Grid = GTK_GRID(gtk_grid_new());

    gtk_grid_set_row_homogeneous(Grid, Desc->RowHomogeneous);
    gtk_grid_set_column_homogeneous(Grid, Desc->ColumnHomogeneous);
    gtk_grid_set_row_spacing(Grid, Desc->RowSpacing);
    gtk_grid_set_column_spacing(Grid, Desc->ColumnSpacing);

    GridTrData* Tr;
    GridTdData* Td;
    int x = 0, y = 0;

    GridTrData* Data = Desc->List;

    while (Data)
    {
        x = 0;
        Tr = Data;
        Td = Tr->Head;
        while (Td)
        {
            gtk_grid_attach(Grid, Td->Widget, x, y, Td->Width, Td->Height);
            Td = Td->Next;
            x++;
        }
        y++;
        Data = Data->Next;
    }

    gtk_widget_show(GTK_WIDGET(Grid));
    return GTK_WIDGET(Grid);
}

void Grid_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
    GridDesc Desc;
    GridDesc_Init(&Desc);

    GridTrData* Tr = NULL;
    GridTdData* Td = NULL;

    XmlAttribute* Attr = XML_GetAttributes(Element);
    while (Attr)
    {
        const char* AttrName = XML_GetAttributeName(Attr);
        if (!strcmp(AttrName, "RowHomo"))
        {
            Desc.RowHomogeneous = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "ColHomo"))
        {
            Desc.RowHomogeneous = XML_GetAttributeValueBool(Attr);
        }
        else if (!strcmp(AttrName, "RowSpace"))
        {
            Desc.RowSpacing = XML_GetAttributeValueInt(Attr);
        }
        else if (!strcmp(AttrName, "ColSpace"))
        {
            Desc.ColumnSpacing = XML_GetAttributeValueInt(Attr);
        }
        Attr = XML_GetNextAttribute(Attr);
    }

    XmlElement* Children = XML_GetElementChildren(Element);
    while (Children)
    {
        const char* ElmName = XML_GetElementName(Children);

        if (!strcmp(ElmName, "GridTr"))
        {
            Td = NULL;
            XmlElement* TdChildren = XML_GetElementChildren(Children);
            while (TdChildren)
            {
                const char* TdElmName = XML_GetElementName(TdChildren);
                if (!strcmp(TdElmName, "GridTd"))
                {
                    WidgetContainer* Container = WidgetContainer_Read(IdList, XML_GetElementChildren(TdChildren), Parent);
                    Parent->Children = WidgetContainer_AddContainer(Parent->Children, Container);

                    int Width = XML_GetAttributeValueInt(XML_FindAttribute(TdChildren, "Width"));
                    int Height = XML_GetAttributeValueInt(XML_FindAttribute(TdChildren, "Height"));

                    Td = GridTdData_Add(Td, Container->Widget, Width, Height);
                }
                TdChildren = XML_GetNextElement(TdChildren);
            }
            Tr = GridTrData_Add(Tr, Td);
        }

        Children = XML_GetNextElement(Children);
    }

    Desc.List = Tr;
    Parent->Widget = Grid_Create(&Desc);
    GridTrData_Free(Tr);
}
