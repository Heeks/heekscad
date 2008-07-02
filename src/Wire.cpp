// Wire.cpp
#include "stdafx.h"
#include "Wire.h"
#include "../interface/Tool.h"
#include "BRepOffsetAPI_MakeOffset.hxx"

wxIcon* CWire::m_icon = NULL;

CWire::CWire(const TopoDS_Wire &wire, const char* title):CShape(wire, title){
}

CWire::~CWire(){
}

wxIcon* CWire::GetIcon(){
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + "/icons/wire.png", wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}

static std::string title_for_OffsetWireTool;

class OffsetWireTool:public Tool{
	CWire* m_wire;
	double m_offset;

public:
	OffsetWireTool(CWire* wire, double offset):m_wire(wire), m_offset(offset){}

	// Tool's virtual functions
	void Run(){
		BRepOffsetAPI_MakeOffset make_operation(m_wire->Wire());
		make_operation.Perform(m_offset);
		HeeksObj* new_object = CShape::MakeObject(make_operation.Shape(), "Result of Wire Offset");
		if(make_operation.Generated(make_operation.Shape()).Extent() > 0){
			wxMessageBox("Generated");
		}

		// ask about generation for each edge
		TopExp_Explorer ex;
		for ( ex.Init( m_wire->Shape(), TopAbs_EDGE ) ; ex.More(); ex.Next() )
		{
			TopoDS_Edge E = TopoDS::Edge(ex.Current());
			if(int extent = make_operation.Generated(E).Extent() > 0){
				char mess[1024];
				sprintf(mess, "Generated from edge = %d", extent);
				wxMessageBox(mess);
			}
		}

		if(make_operation.Modified(make_operation.Shape()).Extent() > 0){
			wxMessageBox("Modified");
		}
		if(make_operation.IsDeleted(make_operation.Shape())){
			wxMessageBox("IsDeleted");
		}
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().DeleteUndoably(m_wire);
	}
	const char* GetTitle(){
		char str[1024];
		sprintf(str, "Offset Wire ( %lf )", m_offset);
		title_for_OffsetWireTool.assign(str);
		return title_for_OffsetWireTool.c_str();
	}
};

void CWire::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	t_list->push_back(new OffsetWireTool(this, 2.0));
	t_list->push_back(new OffsetWireTool(this, -2.0));
}

const TopoDS_Wire &CWire::Wire()const{
	return *((TopoDS_Wire*)(&m_shape));
}