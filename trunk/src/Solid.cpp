// Solid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Solid.h"
#include "BRepBuilderAPI_Transform.hxx"
#include "MarkedList.h"

CSolid::CSolid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col):CShape(solid, title, col)
{
}

CSolid::CSolid(const HeeksColor& col):CShape(col)
{
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

bool CSolid::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	BRepBuilderAPI_Transform myBRepTransformation(m_shape,mat);
	TopoDS_Shape new_shape = myBRepTransformation.Shape();
	CSolid* new_object = new CSolid(*((TopoDS_Solid*)(&new_shape)), m_title.c_str(), m_color);
	new_object->CopyIDsFrom(this);
	wxGetApp().AddUndoably(new_object, m_owner, NULL);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().DeleteUndoably(this);
	return true;
}

void CSolid::OnApplyProperties()
{
	CSolid* new_object = new CSolid(*((TopoDS_Solid*)(&m_shape)), m_title.c_str(), m_color);
	new_object->CopyIDsFrom(this);
	wxGetApp().StartHistory();
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(this);
	wxGetApp().EndHistory();
	wxGetApp().m_marked_list->Clear(true);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().Repaint();
}
