// Wire.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Wire.h"
#include "../interface/Tool.h"
#include "ConversionTools.h"
#include "../interface/HeeksCADInterface.h"

extern CHeeksCADInterface heekscad_interface;

CWire::CWire(const TopoDS_Wire &wire, const wxChar* title):CShape(wire, title, false, 1.0f){
}

CWire::~CWire(){
}

static wxString title_for_OffsetWireTool;

class OffsetWireTool:public Tool{
public:
	double m_offset;
	CWire* m_wire;

	OffsetWireTool(CWire* wire, double offset):m_offset(offset), m_wire(wire){}

	// Tool's virtual functions
	void Run(){
		try
		{
			BRepOffsetAPI_MakeOffset make_operation(m_wire->Wire());
			make_operation.Perform(m_offset);
			HeeksObj* new_object = CShape::MakeObject(make_operation.Shape(), _("Result of Wire Offset"), SOLID_TYPE_UNKNOWN, HeeksColor(234, 123, 89), 1.0f);
			if(make_operation.Generated(make_operation.Shape()).Extent() > 0){
				wxMessageBox(_("Generated"));
			}

			// ask about generation for each edge
			TopExp_Explorer ex;
			for ( ex.Init( m_wire->Shape(), TopAbs_EDGE ) ; ex.More(); ex.Next() )
			{
				TopoDS_Edge E = TopoDS::Edge(ex.Current());
				if(int extent = make_operation.Generated(E).Extent() > 0){
					wxString str = wxString(_("Generated from edge")) + _T(" = ") + wxString::Format(_T("%d"), extent);
					wxMessageBox(str);
				}
			}

			if(make_operation.Modified(make_operation.Shape()).Extent() > 0){
				wxMessageBox(_("Modified"));
			}
			if(make_operation.IsDeleted(make_operation.Shape())){
				wxMessageBox(_("Is Deleted"));
			}
			wxGetApp().Add(new_object, NULL);
			wxGetApp().Remove(m_wire);
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			wxMessageBox(wxString(_("Error making offset")) + _T(": ") + Ctt(e->GetMessageString()));
		}

	}
	const wxChar* GetTitle(){
		wxString str = wxString(_("Offset Wire")) + wxString::Format(_T(" %lf"), m_offset);
		title_for_OffsetWireTool.assign(str);
		return title_for_OffsetWireTool.c_str();
	}
};

static OffsetWireTool offset_wire_out(NULL, 2.0);
static OffsetWireTool offset_wire_in(NULL, -2.0);

const wxBitmap &CWire::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/wire.png")));
	return *icon;
}

void CWire::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	offset_wire_out.m_wire = this;
	offset_wire_in.m_wire = this;
	t_list->push_back(&offset_wire_out);
	t_list->push_back(&offset_wire_in);
}

const TopoDS_Wire &CWire::Wire()const{
	return *((TopoDS_Wire*)(&m_shape));
}

/* static */ HeeksObj *CWire::Sketch( const TopoDS_Wire & wire )
{
    const double deviation = heekscad_interface.GetTolerance();
    HeeksObj *sketch = heekscad_interface.NewSketch();
    for(BRepTools_WireExplorer expEdge(TopoDS::Wire(wire)); expEdge.More(); expEdge.Next())
    {
        const TopoDS_Shape &E = expEdge.Current();
        if(!ConvertEdgeToSketch2(TopoDS::Edge(E), sketch, deviation))return NULL;
    }

    return(sketch);
}


