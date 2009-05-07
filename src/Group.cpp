// Group.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Group.h"
#include "Shape.h"
#include "../tinyxml/tinyxml.h"

void CGroup::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( "Group" );  
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
HeeksObj* CGroup::ReadFromXMLElement(TiXmlElement* element)
{
	CGroup* new_object = new CGroup;

	// instead of ( ObjList:: ) new_object->ReadBaseXML(pElem);

	// loop through all the objects
	for(TiXmlElement* pElem = TiXmlHandle(element).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
	{
		std::string name(pElem->Value());
		if(name == "solid")
		{
			int id = 0;
			pElem->Attribute("id", &id);
			new_object->m_loaded_solid_ids.push_back(id); // solid found after load with CGroup::MoveSolidsToGroupsById
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

// static
void CGroup::MoveSolidsToGroupsById(HeeksObj* object)
{
	std::list<HeeksObj*> objects;
	for(HeeksObj* o = object->GetFirstChild(); o; o = object->GetNextChild())
	{
		objects.push_back(o);
	}

	if(object->GetType() == GroupType)
	{
		CGroup* group = (CGroup*)object;

		for(std::list<int>::iterator It = group->m_loaded_solid_ids.begin(); It != group->m_loaded_solid_ids.end(); It++)
		{
			int id = *It;
			HeeksObj* o = wxGetApp().GetIDObject(SolidType, id);
			o->m_owner->Remove(o);
			group->Add(o, NULL);
		}
	}

	for(std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		MoveSolidsToGroupsById(object);
	}
}
