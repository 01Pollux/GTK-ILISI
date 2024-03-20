#include "TextArea.h"

void TextAreaDesc_Init(TextAreaDesc* Desc)
{
	Desc->Text = NULL;
	Desc->Editable = TRUE;
	Desc->Wrap = GTK_WRAP_WORD;
}

GtkWidget* TextArea_Create(TextAreaDesc* Desc)
{
	GtkWidget* TextArea = gtk_text_view_new();
	if (Desc->Text)
	{
		GtkTextBuffer* Buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(TextArea));
		gtk_text_buffer_set_text(Buf, Desc->Text, (int)strlen(Desc->Text));
	}
	gtk_text_view_set_editable(GTK_TEXT_VIEW(TextArea), Desc->Editable);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(TextArea), Desc->Wrap);
	gtk_widget_show(TextArea);
	return TextArea;
}

void TextArea_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
	TextAreaDesc Desc;
	TextAreaDesc_Init(&Desc);
	XmlAttribute* Attr = XML_GetAttributes(Element);
	const char* AttrName;

	while (Attr)
	{
		AttrName = XML_GetAttributeName(Attr);
		if (!strcmp(AttrName, "Text"))
		{
			Desc.Text = XML_GetAttributeValue(Attr);
		}
		else if (!strcmp(AttrName, "Editable"))
		{
			Desc.Editable = XML_GetAttributeValueBool(Attr);
		}
		else if (!strcmp(AttrName, "Wrap"))
		{
			const char* AttrVal = XML_GetAttributeValue(Attr);
			if (!strcmp(AttrVal, "Word"))
			{
				Desc.Wrap = GTK_WRAP_WORD;
			}
			else if (!strcmp(AttrVal, "Char"))
			{
				Desc.Wrap = GTK_WRAP_CHAR;
			}
			else if (!strcmp(AttrVal, "None"))
			{
				Desc.Wrap = GTK_WRAP_NONE;
			}
			else if (!strcmp(AttrVal, "WordChar"))
			{
				Desc.Wrap = GTK_WRAP_WORD_CHAR;
			}
		}

		Attr = XML_GetNextAttribute(Attr);
	}
	Parent->Widget = TextArea_Create(&Desc);
}
