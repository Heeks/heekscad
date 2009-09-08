// Pad.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Pad.h"
#include "Shape.h"
#include "RemoveOrAddTool.h"

void CPad::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( "Pad" );  
	return;
	root->LinkEndChild( element );

	// instead of ObjList::WriteBaseXML(element), write the id of solids, or the object
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(CShape::IsTypeAShape(object->GetType()))
		{
			TiXmlElement* solid_element = new TiXmlElement( "solid" );
			element->LinkEndChild( solid_element );  
			solid_element->SetAttribute("id", object->m_id);
		}

		object->WriteXML(element);
	}
	HeeksObj::WriteBaseXML(element);
}

// static member function
HeeksObj* CPad::ReadFromXMLElement(TiXmlElement* element)
{
	CPad* new_object = new CPad;
	return new_object;

	// instead of ( ObjList:: ) new_object->ReadBaseXML(pElem);

	// loop through all the objects
	for(TiXmlElement* pElem = TiXmlHandle(element).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
	{
		std::string name(pElem->Value());
		if(name == "solid")
		{
			int id = 0;
			pElem->Attribute("id", &id);
		}
		else
		{
			// load other objects normal
			HeeksObj* object = wxGetApp().ReadXMLElement(pElem);
			if(object)new_object->Add(object, NULL);
		}
	}

	new_object->HeeksObj::ReadBaseXML(element);

	return (ObjList*)new_object;
}

