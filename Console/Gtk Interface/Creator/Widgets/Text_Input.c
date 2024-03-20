#include "Text_Input.h"


/**
* Fonction:	TextInputDesc_Init
*	cette fonction permet d'initialiser la structure de "TextInputDesc"
*	Entree :
* Desc : Description du widget de zone de texte de type "TextInputDesc*"
*	Sortie :
* Desc : Une Structure "TextInputDesc" initialiser par defaut
*/

void TextInputDesc_Init(TextInputDesc* Desc)
{
	Desc->MaxChars = 20; // max des caracteres sur une ligne de zone de texte
	Desc->Visible = TRUE; // les caracteres sont visible ou non
	Desc->StartText = NULL; // le texte entrer dans le debut par defaut 
	Desc->HintText = NULL; // une indice sur le contenu de la zone du texte
}

/**
* Fonction:	TextInput_Create
*	cette fonction permet de creer une zone de texte a l'aide d'une description
*	Entree :
* Desc : Description du widget de zone de texte de type "TextInputDesc*"
*	Sortie :
* TextInput : la zone de texte crée a l'aide de la description
*/
GtkWidget* TextInput_Create(TextInputDesc* Desc)
{
	GtkWidget* TextInput = gtk_entry_new();
	// nombre maximum des caractere dans la zone de texte
	if (Desc->MaxChars != -1)
	{
		gtk_entry_set_width_chars(GTK_ENTRY(TextInput), Desc->MaxChars);
	}
	// la visibilite des caractere de zone du texte
	gtk_entry_set_visibility(GTK_ENTRY(TextInput), Desc->Visible);

	// si on va entrer un texte par defaut 
	if (Desc->StartText)
	{
		gtk_entry_set_text(GTK_ENTRY(TextInput), Desc->StartText);
	}

	// une indice sur le contenu de la zone du text
	if (Desc->HintText)
		gtk_entry_set_placeholder_text(GTK_ENTRY(TextInput), Desc->HintText);

	gtk_widget_show(TextInput);
	return TextInput;
}

/**
* Fonction:	TextInput_Add_Icon
*	cette fonction permet d'ajouter une icon
*	Entree :
* Input : la zone de texte de type "GtkWidget *" qu'on va ajouter l'icon
* Path : l'emplacement de l'icon de type "const gchar*"
*	Sortie :
* TextInput : la zone de texte avec une icon
*/
void TextInput_Add_Icon(GtkWidget* Input, const gchar* Path)
{
	// construire un Gfile a l'aide d'un emplacement donnee
	GFile* File = g_file_new_for_path(Path);
	// la creation d'une icon 
	GIcon* Icon = g_file_icon_new(File);
	//
	gtk_entry_set_icon_from_gicon(GTK_ENTRY(Input), GTK_ENTRY_ICON_PRIMARY, Icon);
}

/**
* Fonction:	TextInput_CreateWithXml
*	cette fonction permet de cree une zone de texte a l'aide de Xml
*	Entre :
* IdList[26] : Tableau qui contient les identificateurs de les balises
* Element : La balise courante de type "XmlElement*"
* Parent : La balise courante de type "WidgetContainer*"
*	Sortie :
* Zone de texte cree a l'aide de fichier xml.
*/
void TextInput_CreateWithXml(WidgetIdList* IdList[26], XmlElement* Element, WidgetContainer* Parent)
{
	TextInputDesc Desc;
	//initialisation de la description du zone de texte
	TextInputDesc_Init(&Desc);

	//prendre le premier attribut du balise
	XmlAttribute* Attr = XML_GetAttributes(Element);
	const char* AttrName;

	//Tantqu'il y a des attributs
	while (Attr)
	{
		//Prendre le nom de l'attribut
		AttrName = XML_GetAttributeName(Attr);
		//si le nom de l'attribut est Maxchars
		if (!strcmp(AttrName, "MaxChars"))
		{
			//Nombre maximum des caractere qu'on va entrer dans la zone de texte
			Desc.MaxChars = XML_GetAttributeValueInt(Attr);
		}
		//si le nom de l'attribut est Visible
		else if (!strcmp(AttrName, "Visible"))
		{
			Desc.Visible = XML_GetAttributeValueBool(Attr);
		}
		//si le nom de l'attribut est StartText
		else if (!strcmp(AttrName, "StartText"))
		{
			Desc.StartText = XML_GetAttributeValue(Attr);
		}
		//si le nom de l'attribut est HintText
		else if (!strcmp(AttrName, "HintText"))
		{
			Desc.HintText = XML_GetAttributeValue(Attr);
		}
		//si le nom de l'attribut est Icon
		else if (!strcmp(AttrName, "Icon"))
		{
			TextInput_Add_Icon(Parent->Widget, XML_GetAttributeValue(Attr));
		}
		Attr = XML_GetNextAttribute(Attr);
	}//fin du while

	Parent->Widget = TextInput_Create(&Desc);
}