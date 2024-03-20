#include "Button.h"
/*
* Prototype: void RadioButtonDesc_Init(RadioButtonDesc* Desc);
* Entree: Pointeur description du Boutton .
* Sortie: Initialisation des valeurs de la structure RadioButtonDesc.
*/
void RadioButtonDesc_Init(RadioButtonDesc* Desc)
{
	//Initialiasation des parametres
	Desc->Label = "Button";
	Desc->Link = NULL;
}

/*
* Prototype: GtkWidget* Button_Create();
* Entree:
* Sortie: GtkWidget*, Le widget Boutton.
* Fonction:Creation d'un boutton simple.
*/
GtkWidget* Button_Create()
{
	GtkWidget* Button = gtk_button_new();
	gtk_widget_show(Button);
	return Button;
}

/*
* Prototype: GtkWidget* Button_Label_Create(const gchar* Label);
* Entree: Chaine de caracteres (label du boutton).
* Sortie: GtkWidget*, Le widget Boutton.
* Fonction:Creation d'un boutton avec un label.
*/
GtkWidget* Button_Label_Create(const gchar* Label)
{
	GtkWidget* Button = gtk_button_new_with_label(Label);
	gtk_widget_show(Button);
	return Button;
}

/*
* Prototype: GtkWidget* Button_Image_Create(const gchar* Path);
* Entree: Chaine de caracteres (Le chemaine de l'image du boutton).
* Sortie: GtkWidget*, Le widget Boutton.
* Fonction:Creation d'un boutton avec une image.
*/
GtkWidget* Button_Image_Create(const gchar* Path)
{
	GtkWidget* Button = gtk_button_new();
	GtkWidget* Image = gtk_image_new_from_file(Path);
	gtk_button_set_image(GTK_BUTTON(Button), Image);
	gtk_widget_show(Button);
	return Button;
}

/*
* Prototype: GtkWidget* Button_Color_Create(const GdkRGBA* Color);
* Entree: La couleur en RGB du boutton.
* Sortie: GtkWidget*, Le widget Boutton.
* Fonction:Creation d'un boutton avec un couleur.
*/
GtkWidget* Button_Color_Create(const GdkRGBA* Color)
{
	GtkWidget* Button = gtk_color_button_new_with_rgba(Color);
	gtk_widget_show(Button);
	return Button;
}

/*
* Prototype: GtkWidget* Button_Radio_Create(RadioButtonDesc* Desc);
* Entree: Pointeur description du Boutton .
* Sortie: GtkWidget*, Le widget Boutton.
* Fonction:Creation d'un boutton de type radio.
*/
GtkWidget* Button_Radio_Create(RadioButtonDesc* Desc)
{
	GtkWidget* Button = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(Desc->Link), Desc->Label);
	gtk_widget_show(Button);
	return Button;
}

/*
* Prototype: void Button_CreateWithXml(WidgetIdList* IdList[26],
			XmlElement* Element,
			WidgetContainer* Parent)
* Entree: Liste des identificateur, Element XML, Conteneur(Parent).
* Sortie: vide.
* Fonction: Creation du boutton a partire du fichier de donnée XML.
*/
void Button_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
	//Chercher l'attribut "Type"
	XmlAttribute* Attr = XML_FindAttribute(Element, "Type");
	GtkWidget* Widget = NULL;
	//Si l'attribut existe;
	if (Attr)
	{
		//Recuperer sa valeur
		const char* Value = XML_GetAttributeValue(Attr);
		//Type Label
		if (!strcmp(Value, "Label"))
		{	//Chercher l'attribut "Text"
			Attr = XML_FindAttribute(Element, "Text");
			Value = "";
			//S'il existe
			if (Attr)
			{	//Recuperer sa valeur
				Value = XML_GetAttributeValue(Attr);
			}
			//Creation du button avec label
			Widget = Button_Label_Create(Value);
		}//Type image
		else if (!strcmp(Value, "Image"))
		{	//Rechercher l'attribut "Path"
			Attr = XML_FindAttribute(Element, "Path");
			Value = "";
			//S'il existe
			if (Attr)
			{	//Recuperer sa valeur
				Value = XML_GetAttributeValue(Attr);
			}
			//Creation du button avec image
			Widget = Button_Image_Create(Value);
		}//Type Color
		else if (!strcmp(Value, "Color"))
		{	//Rechercher l'attribut "Rgba"
			Attr = XML_FindAttribute(Element, "Rgba");
			Value = "";
			//Initialisation de la couleur du boutton
			GdkRGBA Color = { 1.0, 1.0, 1.0, 1.0 };
			//Si l'attribut existe
			if (Attr)
			{	//Recuperer sa valeur
				XML_GetAttributeValueFormat(Attr, "%d,%d,%d,%d", &Color.red, &Color.green, &Color.blue, &Color.alpha);
			}
			//Creation du button avec couleur
			Widget = Button_Color_Create(&Color);
		}//Type radio
		else if (!strcmp(Value, "Radio"))
		{
			RadioButtonDesc Desc;
			RadioButtonDesc_Init(&Desc);
			//Recuperer la liste des attributs
			Attr = XML_GetAttributes(Element);
			while (Attr)
			{	//Recuperer le nom de l'attribut
				const char* AttrName = XML_GetAttributeName(Attr);
				//attribut :"LinkId"
				if (!strcmp(AttrName, "LinkId"))
				{	//Chercher le widget du meme id que le button
					WidgetContainer* Container = WidgetIdList_GetElementById(IdList, XML_GetAttributeValue(Attr));
					//S'il existe						XML_GetAttributeValue(Attr));
					if (Container)
					{	//lier widget avec le boutton
						Desc.Link = Container->Widget;
					}
				}//attribut :"Text"
				else if (!strcmp(AttrName, "Text"))
				{	//Initialiser le label dans la structure
					Desc.Label = XML_GetAttributeValue(Attr);
				}
				//Passer au attribut suivant
				Attr = XML_GetNextAttribute(Attr);
			}
			//Creation du button de type radio
			Widget = Button_Radio_Create(&Desc);
		}
	}
	//Boutton Simple
	else
	{
		Widget = Button_Create();
	}

	Parent->Widget = Widget;
}

/*
* Prototype: GtkWidget* CheckBox_Create(const gchar* label);
* Entree: Chaine de caracteres (label du boutton).
* Sortie: GtkWidget*, Le widget Boutton.
* Fonction:Creation d'un boutton de type checkbox.
*/
GtkWidget* CheckBox_Create(const gchar* label)
{
	GtkWidget* CheckBox = gtk_check_button_new_with_label(label);
	return CheckBox;
}