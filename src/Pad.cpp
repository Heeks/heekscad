// Pad.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Pad.h"
#include "Shape.h"
#include "RuledSurface.h"
#include "RemoveOrAddTool.h"
#include "../interface/PropertyDouble.h"

CPad::CPad(double length)
{
	m_length = length;
}

CPad::CPad()
{
	m_length = 0;
}

void CPad::glCommands(bool select, bool marked, bool no_color)
{
	//Draw everything else
	ObjList::glCommands(select,marked,no_color);

	//TODO: only do this when the sketch is dirty

	//Get the sketch
	CSketch* sketch = dynamic_cast<CSketch*>(GetFirstChild());
	if(!sketch)
		return;

	std::vector<TopoDS_Face> faces = sketch->GetFaces();
	std::list<TopoDS_Face> facelist(faces.begin(),faces.end());
	std::list<TopoDS_Shape> new_shapes;
	CreateExtrusions(facelist, new_shapes, gp_Vec(0, 0, m_length).Transformed(wxGetApp().GetDrawMatrix(false)));

	//This is a pretty ugly hack
	std::list<TopoDS_Shape>::iterator it;
	for(it = new_shapes.begin(); it != new_shapes.end(); ++it)
	{
		CShape shape(*it,_("Argh"),wxGetApp().current_color);
		shape.glCommands(select,marked,no_color);
	}

}

void OnSetHeight(double b, HeeksObj* o)
{
	((CPad*)o)->m_length = b;
	wxGetApp().Repaint();
}

void CPad::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyDouble(_("Height"), m_length, this,OnSetHeight));

	ObjList::GetProperties(list);
}

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

// static
void CPad::PadSketch(CSketch* sketch, double length, bool undoably)
{
	CPad *pad = new CPad(length);
	if(undoably)
	{
		wxGetApp().DoToolUndoably(new AddObjectTool(pad, sketch->Owner(),NULL));

		wxGetApp().DoToolUndoably(new RemoveObjectTool(sketch));

		wxGetApp().DoToolUndoably(new AddObjectTool(sketch, pad,NULL));

		//The Add/Remove tools are fubar, so we need to mark all the sketches children as modified to get it to show up in the tree
		HeeksObj* child = sketch->GetFirstChild();
		while(child)
		{
			wxGetApp().WasModified(child);
			child = sketch->GetNextChild();
		}
	}
	else
	{
		sketch->Owner()->Add(pad,NULL);

		sketch->Owner()->Remove(sketch);
		sketch->RemoveOwner(sketch->Owner());
		pad->Add(sketch,NULL);
	}
}
