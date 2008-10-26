// Solid.cpp
#include "stdafx.h"
#include "Solid.h"
#include "BRepBuilderAPI_Transform.hxx"
#include "MarkedList.h"

wxIcon* CSolid::m_icon = NULL;

CSolid::CSolid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col):CShape(solid, title, col)
{
}

CSolid::~CSolid()
{
}

wxIcon* CSolid::GetIcon()
{
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + _T("/icons/solid.png"), wxBITMAP_TYPE_PNG);
	}
	return m_icon;
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
	wxGetApp().AddUndoably(new_object, m_owner, NULL);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().DeleteUndoably(this);
	return true;
}