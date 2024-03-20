#include "Image.h"
#include <math.h>

void ImageDesc_Init(ImageDesc* Desc)
{
	Desc->Path = NULL;
	Desc->UseIcon = FALSE;
	Desc->Width = 0;
	Desc->Height = 0;
	Desc->Type = GTK_ICON_SIZE_BUTTON;
}

GtkWidget* Image_Create(ImageDesc* Desc)
{
	GtkWidget* Image = NULL;
	if (Desc->UseIcon)
	{
		Image = gtk_image_new_from_icon_name(Desc->Path, Desc->Type);
	}
	else
	{
		GdkPixbuf* Buf = gdk_pixbuf_new_from_file_at_scale(Desc->Path, Desc->Width, Desc->Height, TRUE, NULL);
		Image = gtk_image_new_from_pixbuf(Buf);
	}
	gtk_widget_show(Image);
	return Image;
}

void Image_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
	ImageDesc Desc;
	ImageDesc_Init(&Desc);

	XmlAttribute* Attr = XML_GetAttributes(Element);
	const char* AttrName;

	while (Attr)
	{
		AttrName = XML_GetAttributeName(Attr);
		if (!strcmp(AttrName, "Path"))
		{
			Desc.Path = XML_GetAttributeValue(Attr);
		}
		else if (!strcmp(AttrName, "UseIcon"))
		{
			Desc.UseIcon = XML_GetAttributeValueBool(Attr);
		}
		else if (!strcmp(AttrName, "Size"))
		{
			XML_GetAttributeValueFormat(Attr, "%dx%d", &Desc.Width, &Desc.Height);
		}
		else if (!strcmp(AttrName, "Type"))
		{
			const char* Type = XML_GetAttributeValue(Attr);
			if (!strcmp(Type, "Button"))
				Desc.Type = GTK_ICON_SIZE_BUTTON;
			else if (!strcmp(Type, "Menu"))
				Desc.Type = GTK_ICON_SIZE_MENU;
			else if (!strcmp(Type, "SmallToolbar"))
				Desc.Type = GTK_ICON_SIZE_SMALL_TOOLBAR;
			else if (!strcmp(Type, "LargeToolbar"))
				Desc.Type = GTK_ICON_SIZE_LARGE_TOOLBAR;
			else if (!strcmp(Type, "Dnd"))
				Desc.Type = GTK_ICON_SIZE_DND;
			else if (!strcmp(Type, "Dialog"))
				Desc.Type = GTK_ICON_SIZE_DIALOG;
		}
		Attr = XML_GetNextAttribute(Attr);
	}
	Parent->Widget = Image_Create(&Desc);
}
