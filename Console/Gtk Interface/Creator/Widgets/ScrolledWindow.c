#include "ScrolledWindow.h"

void ScrolledWindowDesc_Init(ScrolledWindowDesc* Desc)
{
	Desc->HAdjustment.Active = FALSE;
	Desc->VAdjustment.Active = FALSE;

	Desc->Corner = GTK_CORNER_TOP_LEFT;
	Desc->HAdjustment.Visibility = GTK_POLICY_AUTOMATIC;
	Desc->VAdjustment.Visibility = GTK_POLICY_AUTOMATIC;
}

GtkWidget* ScrolledWindow_Create(ScrolledWindowDesc* Desc)
{
	GtkAdjustment* HAdjustment = NULL, * VAdjustment = NULL;
	if (Desc->HAdjustment.Active)
	{
		HAdjustment = gtk_adjustment_new(
			Desc->HAdjustment.Value,
			Desc->HAdjustment.Lower,
			Desc->HAdjustment.Upper,
			Desc->HAdjustment.StepIncrement,
			Desc->HAdjustment.PageIncrement,
			Desc->HAdjustment.PageSize);
	}
	if (Desc->VAdjustment.Active)
	{
		VAdjustment = gtk_adjustment_new(
			Desc->VAdjustment.Value,
			Desc->VAdjustment.Lower,
			Desc->VAdjustment.Upper,
			Desc->VAdjustment.StepIncrement,
			Desc->VAdjustment.PageIncrement,
			Desc->VAdjustment.PageSize);
	}

	GtkWidget* Scrolled = gtk_scrolled_window_new(HAdjustment, VAdjustment);
	gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(Scrolled), Desc->Corner);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Scrolled), Desc->HAdjustment.Visibility, Desc->VAdjustment.Visibility);
	gtk_widget_show(Scrolled);
	return Scrolled;
}

void ScrolledWindow_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
	ScrolledWindowDesc Desc;
	ScrolledWindowDesc_Init(&Desc);
	XmlAttribute* Attr = XML_GetAttributes(Element);
	const char* AttrName;

	while (Attr)
	{
		AttrName = XML_GetAttributeName(Attr);
		if (!strcmp(AttrName, "HAdjustment"))
		{
			Desc.HAdjustment.Active = XML_GetAttributeValueBool(Attr);
		}
		else if (!strcmp(AttrName, "VAdjustment"))
		{
			Desc.VAdjustment.Active = XML_GetAttributeValueBool(Attr);
		}
		else if (!strcmp(AttrName, "HVisibility"))
		{
			Desc.HAdjustment.Visibility = XML_GetAttributeValueBool(Attr) ? GTK_POLICY_ALWAYS : GTK_POLICY_NEVER;
		}
		else if (!strcmp(AttrName, "VVisibility"))
		{
			Desc.VAdjustment.Visibility = XML_GetAttributeValueBool(Attr) ? GTK_POLICY_ALWAYS : GTK_POLICY_NEVER;
		}
		else if (!strcmp(AttrName, "Corner"))
		{
			Desc.Corner = XML_GetAttributeValueBool(Attr) ? GTK_CORNER_BOTTOM_LEFT : GTK_CORNER_TOP_LEFT;
		}
		else if (!strcmp(AttrName, "HValue"))
		{
			Desc.HAdjustment.Value = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "VValue"))
		{
			Desc.VAdjustment.Value = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "HLower"))
		{
			Desc.HAdjustment.Lower = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "VLower"))
		{
			Desc.VAdjustment.Lower = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "HUpper"))
		{
			Desc.HAdjustment.Upper = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "VUpper"))
		{
			Desc.VAdjustment.Upper = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "HStepIncrement"))
		{
			Desc.HAdjustment.StepIncrement = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "VStepIncrement"))
		{
			Desc.VAdjustment.StepIncrement = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "HPageIncrement"))
		{
			Desc.HAdjustment.PageIncrement = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "VPageIncrement"))
		{
			Desc.VAdjustment.PageIncrement = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "HPageSize"))
		{
			Desc.HAdjustment.PageSize = XML_GetAttributeValueFloat(Attr);
		}
		else if (!strcmp(AttrName, "VPageSize"))
		{
			Desc.VAdjustment.PageSize = XML_GetAttributeValueFloat(Attr);
		}
		Attr = XML_GetNextAttribute(Attr);
	}

	Parent->Widget = ScrolledWindow_Create(&Desc);
	WidgetContainer_ReadChildrens(IdList, Element, Parent);

	WidgetContainer* Child = Parent->Childrens;
	while (Child)
	{
		if (Child->Widget)
		{
			gtk_container_add(GTK_CONTAINER(Parent->Widget), Child->Widget);
		}
		Child = Child->Next;
	}
}
