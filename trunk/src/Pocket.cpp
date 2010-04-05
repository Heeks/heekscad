// Pocket.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Pocket.h"
#include "Shape.h"
#include "RuledSurface.h"
#include "../interface/PropertyDouble.h"
#include "Part.h"

CPocket::CPocket(double length)
{
	m_length = length;
	m_faces->m_visible=false;
}

CPocket::CPocket()
{
	m_length = 0;
}

bool CPocket::IsDifferent(HeeksObj* other)
{
	CPocket* pocket = (CPocket*)other;
	if(pocket->m_length != m_length)
		return true;

	return HeeksObj::IsDifferent(other);
}

void CPocket::ReloadPointers()
{
	DynamicSolid::ReloadPointers();

	HeeksObj *child = GetFirstChild();
	while(child)
	{
		CSketch* sketch = dynamic_cast<CSketch*>(child);
		if(sketch)
		{
			m_sketch = sketch;
			break;
		}
		child = GetNextChild();
	}

	Update();
}

gp_Trsf CPocket::GetTransform()
{
	if(m_sketch && m_sketch->m_coordinate_system)
		return m_sketch->m_coordinate_system->GetMatrix();
	return gp_Trsf();
}

void CPocket::Update()
{
	if(m_sketch)
	{
		std::vector<TopoDS_Face> faces = m_sketch->GetFaces();
		std::list<TopoDS_Shape> facelist(faces.begin(),faces.end());
		std::list<TopoDS_Shape> new_shapes;
		CreateExtrusions(facelist, new_shapes, gp_Vec(0, 0, m_length));

		SetShapes(new_shapes);
	}

	DynamicSolid* solid = dynamic_cast<DynamicSolid*>(Owner());
	if(solid)
		solid->Update();
}

void CPocket::glCommands(bool select, bool marked, bool no_color)
{
	//TODO: only do this when the sketch is dirty

	glPushMatrix();
	if(m_sketch)
	{
		Update();
		if(m_sketch->m_coordinate_system)
			m_sketch->m_coordinate_system->ApplyMatrix();
//		DrawShapes();
	}

	//Draw everything else
	ObjList::glCommands(select,marked,no_color);
	glPopMatrix();

}

void OnPocketSetHeight(double b, HeeksObj* o)
{
	((CPocket*)o)->m_length = b;
	wxGetApp().Repaint();
}

void CPocket::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyDouble(_("Height"), m_length, this,OnPocketSetHeight));

	ObjList::GetProperties(list);
}

void CPocket::WriteXML(TiXmlNode *root)
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
HeeksObj* CPocket::ReadFromXMLElement(TiXmlElement* element)
{
	CPocket* new_object = new CPocket;
	return new_object;

#if 0
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
#endif
}

// static
void CPocket::PocketSketch(CSketch* sketch, double length)
{
	CPocket *pad = new CPocket(length);
	sketch->Owner()->Add(pad,NULL);

	sketch->Owner()->Remove(sketch);
	sketch->RemoveOwner(sketch->Owner());
	sketch->m_draw_with_transform = false;
	pad->Add(sketch,NULL);
	pad->ReloadPointers();
}
