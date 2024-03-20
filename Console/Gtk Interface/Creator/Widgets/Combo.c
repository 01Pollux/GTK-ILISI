#include "Combo.h"

/*
* Prototype: ComboBoxItem* ComboBoxItem_Add(ComboBoxItem* Head,
			const gchar* Label);
* Entree: Pointeur sur la tete du Liste Deroulante,Un label.
* Sortie: La tete du liste deroulante.
* Fonction: Ajouter un element a la liste deroulante.
*/
ComboBoxItem* ComboBoxItem_Add(ComboBoxItem* Head, const gchar* Label)
{	//Allocation de la memoire pour le nouveau element
	ComboBoxItem* Tail = (ComboBoxItem*)calloc(1, sizeof(ComboBoxItem));
	if (!Tail)
	{
		printf("Erreur d'allocation\n");
		exit(-1);
	}
	//Initialisation
	Tail->Label = Label;
	//La liste vide
	if (!Head)
	{
		return Tail;
	}
	ComboBoxItem* Tmp = Head;
	//Parcourire la liste
	while (Tmp->Next)
	{
		Tmp = Tmp->Next;
	}
	//ajout de l'element a la fin de la liste
	Tmp->Next = Tail;
	return Head;
}
/*
* Prototype: void ComboBoxItem_Free(ComboBoxItem* Head);
* Entree: Pointeur sur la tete du Liste Deroulante.
* Sortie:
* Fonction: Destruction de la liste deroulant.
*/
void ComboBoxItem_Free(ComboBoxItem* Head)
{
	ComboBoxItem* Tmp;
	//Tant que la liste n'est pas vide
	while (Head)
	{
		Tmp = Head;
		Head = Head->Next;
		free(Tmp);
	}
}

/*
* Prototype: GtkWidget* ComboBox_Create(const ComboBoxItem* Items);
* Entree: Pointeur sur la Liste Deroulante.
* Sortie: Le widget du liste deroulante.
* Fonction: Creation d'une liste deroulante.
*/
GtkWidget* ComboBox_Create(const ComboBoxItem* Items)
{
	GtkWidget* Combo = gtk_combo_box_text_new();
	gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(Combo), 0);
	gtk_widget_show(Combo);
	//Tant qu'il y a des elements dans la liste on les ajoutes au widget du LD
	while (Items)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(Combo), Items->Label);
		Items = Items->Next;
	}
	return Combo;
}

/*
* Prototype: void ComboBox_CreateFromXml(WidgetIdList* IdList[26],
			XmlElement* Element,
			WidgetContainer* Parent);
* Entree: Liste des identificateur, Element XML, Conteneur(Parent).
* Sortie: vide
* Fonction: Creation de Liste Deroulante a partire du fichier de donnée XML.
*/
void ComboBox_CreateFromXml(WidgetIdList* IdList[26],
	XmlElement* Element,
	WidgetContainer* Parent)
{
	ComboBoxItem* Items = NULL;
	//Recuperer les fils de l'elemt xml
	XmlElement* Elm = XML_GetElementChildrens(Element);


	int ActiveId = -1;
	//Chercher l'attribut "Active"
	XmlAttribute* Attr = XML_FindAttribute(Element, "Active");
	//Si l'attribut existe
	if (Attr)
	{	//Recuperer sa valeur
		ActiveId = XML_GetAttributeValueInt(Attr);
	}
	//Tant qu'il y a des elements
	while (Elm)
	{
		if (!strcmp(XML_GetElementName(Elm), "ComboBoxItem"))
		{
			Attr = XML_FindAttribute(Elm, "Text");
			if (Attr)
			{	//Ajouter l'element a la liste deroulante
				Items = ComboBoxItem_Add(Items, XML_GetAttributeValue(Attr));
			}
		}
		//Passer a l'element suivant
		Elm = XML_GetNextElement(Elm);
	}

	//Creation du widget de la liste deroulante
	GtkWidget* Combo = ComboBox_Create(Items);
	if (ActiveId != -1)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(Combo), ActiveId);
	}

	ComboBoxItem_Free(Items);
	Parent->Widget = Combo;
}