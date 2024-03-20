#pragma once

typedef struct XmlAttribute XmlAttribute;
typedef struct XmlElement   XmlElement;
typedef struct XmlDocument  XmlDocument;

XmlDocument* XML_OpenFile(const char* FileName);

void XML_Free(XmlDocument* Document);

XmlElement* XML_GetFirstElement(XmlDocument* Document);

XmlElement* XML_GetNextElement(XmlElement* Element);

XmlElement* XML_FindElement(XmlElement* Element, const char* Name);

//

const char* XML_GetElementName(XmlElement* Element);

XmlElement* XML_GetElementChildren(XmlElement* Element);

XmlAttribute* XML_GetAttributes(XmlElement* Element);

XmlAttribute* XML_FindAttribute(XmlElement* Element, const char* Name);

//

XmlAttribute* XML_GetNextAttribute(XmlAttribute* Attribute);

const char* XML_GetAttributeName(XmlAttribute* Attribute);

const char* XML_GetAttributeValue(XmlAttribute* Attribute);

void XML_GetAttributeValueFormat(XmlAttribute* Attribute, const char* Format, ...);

int XML_GetAttributeValueBool(XmlAttribute* Attribute);

int XML_GetAttributeValueInt(XmlAttribute* Attribute);

float XML_GetAttributeValueFloat(XmlAttribute* Attribute);
