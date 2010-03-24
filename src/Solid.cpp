// Solid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Solid.h"
#include "MarkedList.h"

CSolid::CSolid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col):CShape(solid, title, col)
{
}

CSolid::CSolid(const HeeksColor& col):CShape(col)
{
}

CSolid::CSolid( const CSolid & rhs ) : CShape(rhs)
{
    *this = rhs;    // Call the assignment operator.
}

CSolid::~CSolid()
{
}

HeeksObj *CSolid::MakeACopy(void)const
{
	return new CSolid(*this);
}

void CSolid::SetXMLElement(TiXmlElement* element)
{
	element->SetAttribute("col", m_color.COLORREF_color());
}

void CSolid::SetFromXMLElement(TiXmlElement* pElem)
{
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){m_color = HeeksColor(a->IntValue());}
	}
}

void CSolid::MakeTransformedShape(const gp_Trsf &mat)
{
	BRepBuilderAPI_Transform myBRepTransformation(m_shape,mat);
	m_shape = myBRepTransformation.Shape();
}

void CSolid::OnApplyProperties()
{
	CSolid* new_object = new CSolid(*((TopoDS_Solid*)(&m_shape)), m_title.c_str(), m_color);
	new_object->CopyIDsFrom(this);
	Owner()->Add(new_object, NULL);
	Owner()->Remove(this);
	wxGetApp().m_marked_list->Clear(true);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().Repaint();
}
