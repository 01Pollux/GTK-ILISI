
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "XmlReader.h"

typedef struct XmlAttribute
{
	char* Key;
	char* Value;
	struct XmlAttribute* NextAttribute;
} XmlAttribute;

typedef struct XmlElement
{
	char* Name;
	XmlAttribute* Attributes;
	struct XmlElement* Childrens;
	struct XmlElement* NextElement;
} XmlElement;

typedef struct XmlDocument
{
	XmlElement* Elements;
	char* FileData;
} XmlDocument;


static XmlElement* XML_AddToRoot(XmlElement* Root, char* Name, XmlElement* Childrens, XmlAttribute* Attributes)
{
	XmlElement* Element = (XmlElement*)calloc(1, sizeof(XmlElement));
	if (!Element)
	{
		printf("Erreur d'allocation.");
		exit(-1);
	}

	Element->Name = Name;
	Element->Attributes = Attributes;
	Element->Childrens = Childrens;

	if (!Root)
	{
		Root = Element;
	}
	else
	{
		XmlElement* Tmp = Root;
		while (Tmp->NextElement)
		{
			Tmp = Tmp->NextElement;
		}
		Tmp->NextElement = Element;
	}
	return Root;
}

static XmlAttribute* XML_AddAttribute(XmlAttribute* Attribute, char* Key, char* Value)
{
	XmlAttribute* Element = (XmlAttribute*)calloc(1, sizeof(XmlAttribute));
	if (!Element)
	{
		printf("Erreur d'allocation.");
		exit(-1);
	}
	size_t Size = strlen(Value);
	if (*Value != '"' || (Value[Size - 1] != '"'))
	{
		printf("Invalid value added to attribute: %s::%s\n", Key, Value);
	}
	else
	{
		size_t i;
		for (i = 0; i < Size - 2; i++)
		{
			Value[i] = Value[i + 1];
		}
		Value[i] = '\0';
	}

	Element->Key = Key;
	Element->Value = Value;
	Element->NextAttribute = Attribute;
	return Element;
}

char* XML_ProcessString(
	char* Str,
	int* InsideString,
	char* BreakLoop)
{
	switch (*Str)
	{
	case '"':
		if (!(*InsideString & 0b10))
		{
			*InsideString ^= 0b01;
		}
		break;
	case '\'':
		if (!(*InsideString & 0b01))
		{
			*InsideString ^= 0b10;
		}
		break;

	case ' ':
	{
		if (!*InsideString)
		{
			*BreakLoop = 1;
		}
		break;
	}

	case '>':
	{
		if (!*InsideString)
		{
			*BreakLoop = 2;
		}
		break;
	}
	}
	return Str;
}

static char* JumpToEndOfWord(char* Str, char* BreakLoop)
{
	int InsideString = 0;
	*BreakLoop = 0;

	while (*Str != '\0')
	{
		Str = XML_ProcessString(Str, &InsideString, BreakLoop);
		if (*BreakLoop)
		{
			break;
		}
		++Str;
	}
	*Str = '\0';
	return Str;
}

static char* XML_JumpWhiteSpaces(char* Str)
{
	while (*Str == ' ' || *Str == '\t' || *Str == '\n')
	{
		++Str;
	}
	return Str;
}


static char* XML_ProcessAttributes(char* Str, XmlAttribute** Attributes)
{
	int InsideString = 0;
	char BreakLoop = 0;

	int KeyValue = 0;
	int Error = 0;

	while (*Str != '\0')
	{
		Str = XML_ProcessString(Str, &InsideString, &BreakLoop);
		if (BreakLoop)
		{
			break;
		}
		if (!Error)
		{
			if (isalpha(*Str))
			{
				char* Key = Str;
				// Seek out the key part of string
				while (isalpha(*Str))
				{
					++Str;
				}

				*Str = '\0';
				Str = XML_JumpWhiteSpaces(Str + 1);
				if (*Str == '=')
				{
					Str = XML_JumpWhiteSpaces(Str + 1);
				}

				char* Value = Str;

				// Seek out the value part in string
				while (*Str != '\0')
				{
					Str = XML_ProcessString(Str, &InsideString, &BreakLoop);
					if (BreakLoop)
						break;
					++Str;
				}

				if (BreakLoop != 1)
				{
					char* Tmp = XML_ProcessString(Str + 1, &InsideString, &BreakLoop);
					*Str = '\0';
					Str = Tmp;
					if (BreakLoop == 2)
					{
						BreakLoop = 1;
					}
				}
				else
				{
					*Str = '\0';
					BreakLoop = 0;
				}

				*Attributes = XML_AddAttribute(*Attributes, Key, Value);
				if (BreakLoop == 1)
				{
					break;
				}
			}
		}
		++Str;
	}

	return Str;
}

static char* XML_ProcessChildrens(char* Str, XmlElement** RootElement, char* RootName)
{
	char* Token = Str;

	int InsideString = 0;
	char BreakLoop = 0;

	XmlElement* Element = NULL;

	while (*Str != '\0')
	{
		if (*Str == '<')
		{
			char* Token = ++Str;
			if (*Token == '/' && !strncmp(Token + 1, RootName, strlen(RootName)))
			{
				Token += 1 + strlen(RootName);
				break;
			}

			Str = JumpToEndOfWord(Str, &BreakLoop);

			XmlAttribute* Attributes = NULL;
			if (BreakLoop != 2)
			{
				Str = XML_ProcessAttributes(++Str, &Attributes);
			}

			XmlElement* Childrens = NULL;
			Str = XML_ProcessString(Str, &InsideString, &BreakLoop);
			if (BreakLoop == 2)
			{
				++Str;
			}

			Str = XML_ProcessChildrens(Str, &Childrens, Token);
			Element = XML_AddToRoot(Element, Token, Childrens, Attributes);
		}
		else
		{
			++Str;
		}
	}

	*RootElement = Element;
	return Str;
}


XmlDocument* XML_OpenFile(const char* FileName)
{
	FILE* File = fopen(FileName, "r");
	if (!File)
	{
		return NULL;
	}

	XmlDocument* Document = (XmlDocument*)calloc(1, sizeof(XmlDocument));
	if (!Document)
	{
		printf("Erreur d'allocation.");
		exit(-1);
	}

	fseek(File, 0, SEEK_END);
	size_t FileSize = ftell(File);
	fseek(File, 0, SEEK_SET);

	char* FileData = malloc(FileSize + 1);
	if (!FileData)
	{
		printf("Erreur d'allocation.");
		exit(-1);
	}
	FileData[FileSize] = '\0';
	fread(FileData, 1, FileSize, File);

	fclose(File);

	Document->FileData = FileData;

	char* Str = FileData;
	char* StrEnd = Str + FileSize;

	int InsideString = 0;
	char BreakLoop = 0;

	XmlElement* RootElement = NULL;

	while (Str < StrEnd)
	{
		if (*Str == '<')
		{
			char* Token = ++Str;
			Str = JumpToEndOfWord(Str, &BreakLoop);
			XmlAttribute* Attributes = NULL;
			if (BreakLoop != 2)
			{
				Str = XML_ProcessAttributes(++Str, &Attributes);
			}

			XmlElement* Childrens = NULL;
			Str = XML_ProcessString(Str, &InsideString, &BreakLoop);
			Str = XML_ProcessChildrens(Str + 1, &Childrens, Token);
			RootElement = XML_AddToRoot(RootElement, Token, Childrens, Attributes);

			break;
		}
		else
		{
			Str++;
		}
	}

	Document->Elements = RootElement;
	return Document;
}

void XML_Free_Impl(XmlElement* Root)
{
	XmlElement* Tmp;
	while (Root)
	{

		XmlAttribute* Attribute = Root->Attributes, * TmpAttr;
		while (Attribute)
		{
			TmpAttr = Attribute;
			Attribute = Attribute->NextAttribute;
			free(TmpAttr);
		}

		Tmp = Root;
		Root = Root->NextElement;

		XML_Free_Impl(Tmp->Childrens);
		free(Tmp);
	}
}

void XML_Free(XmlDocument* Document)
{
	XML_Free_Impl(Document->Elements);
	free(Document->FileData);
}

//

XmlElement* XML_GetFirstElement(XmlDocument* Document)
{
	return Document->Elements ? Document->Elements->Childrens : NULL;
}

XmlElement* XML_GetNextElement(XmlElement* Element)
{
	return Element->NextElement;
}

XmlElement* XML_FindElement(XmlElement* Element, const char* Name)
{
	while (Element)
	{
		if (!strcmp(Element->Name, Name))
		{
			break;
		}
		Element = XML_GetNextElement(Element);
	}
	return Element;
}

//

const char* XML_GetElementName(XmlElement* Element)
{
	return Element->Name;
}

XmlElement* XML_GetElementChildrens(XmlElement* Element)
{
	return Element->Childrens;
}

XmlAttribute* XML_GetAttributes(XmlElement* Element)
{
	return Element->Attributes;
}

XmlAttribute* XML_FindAttribute(XmlElement* Element, const char* Name)
{
	XmlAttribute* Attribute = XML_GetAttributes(Element);
	while (Attribute)
	{
		if (!strcmp(Attribute->Key, Name))
		{
			break;
		}
		Attribute = XML_GetNextAttribute(Attribute);
	}
	return Attribute;
}

//

XmlAttribute* XML_GetNextAttribute(XmlAttribute* Attribute)
{
	return Attribute->NextAttribute;
}

const char* XML_GetAttributeName(XmlAttribute* Attribute)
{
	return Attribute->Key;
}

const char* XML_GetAttributeValue(XmlAttribute* Attribute)
{
	return Attribute->Value;
}

void XML_GetAttributeValueFormat(XmlAttribute* Attribute, const char* Format, ...)
{
	va_list Args;
	va_start(Args, Format);
	vsscanf(Attribute->Value, Format, Args);
	va_end(Args);
}

int XML_GetAttributeValueBool(XmlAttribute* Attribute)
{
	char Tmp[5] = { 0 };
	XML_GetAttributeValueFormat(Attribute, "%4s", Tmp);
	return !strncmp(Tmp, "true", 4);
}

int XML_GetAttributeValueInt(XmlAttribute* Attribute)
{
	int Val;
	XML_GetAttributeValueFormat(Attribute, "%d", &Val);
	return Val;
}

float XML_GetAttributeValueFloat(XmlAttribute* Attribute)
{
	float Val;
	XML_GetAttributeValueFormat(Attribute, "%f", &Val);
	return Val;
}

//