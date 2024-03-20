#include "Label.h"

void LabelDesc_Init(LabelDesc* Desc)
{
    Desc->Text = "";
    Desc->Wrap = FALSE;
}

GtkWidget* Label_Create(LabelDesc* Desc)
{
    GtkWidget* Text = gtk_label_new(Desc->Text);
    gtk_label_set_line_wrap(GTK_LABEL(Text), Desc->Wrap);
    gtk_widget_show(Text);
    return Text;
}

void Label_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
    LabelDesc Desc;
    LabelDesc_Init(&Desc);
    ;
    XmlAttribute* Attr = XML_GetAttributes(Element);
    const char*   AttrName;

    while (Attr)
    {
        AttrName = XML_GetAttributeName(Attr);
        if (!strcmp(AttrName, "Text"))
        {
            Desc.Text = XML_GetAttributeValue(Attr);
        }
        else if (!strcmp(AttrName, "Wrap"))
        {
            Desc.Wrap = XML_GetAttributeValueBool(Attr);
        }
        Attr = XML_GetNextAttribute(Attr);
    }
    Parent->Widget = Label_Create(&Desc);
}
