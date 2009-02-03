// Sketch.cpp

#include "stdafx.h"
#include "Sketch.h"
#include "../interface/PropertyInt.h"
#include "../tinyxml/tinyxml.h"

CSketch::CSketch()
{
}

CSketch::CSketch(const CSketch& c)
{
	operator=(c);
}

CSketch::~CSketch()
{
}

const CSketch& CSketch::operator=(const CSketch& c)
{
	// just copy all the lines and arcs, not the id
	ObjList::operator =(c);

	return *this;
}

void CSketch::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyInt(_("Number of elements"), ObjList::GetNumChildren(), this));

	ObjList::GetProperties(list);
}

HeeksObj *CSketch::MakeACopy(void)const
{
	return (ObjList*)(new CSketch(*this));
}

void CSketch::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( "Sketch" );  
	root->LinkEndChild( element );
	WriteBaseXML(element);
}

// static member function
HeeksObj* CSketch::ReadFromXMLElement(TiXmlElement* pElem)
{
	CSketch* new_object = new CSketch;
	new_object->ReadBaseXML(pElem);

	return (ObjList*)new_object;
}

