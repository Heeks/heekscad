// ObjPropsCanvas.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "ObjPropsCanvas.h"
#include "../interface/Property.h"
#include "../interface/ToolImage.h"
#include "../interface/PropertyVertex.h"
#include "propgrid.h"
#include "HeeksFrame.h"
#include "MarkedList.h"
#include "../interface/MarkedObject.h"

BEGIN_EVENT_TABLE(CObjPropsCanvas, wxScrolledWindow)
	EVT_SIZE(CObjPropsCanvas::OnSize)

        // This occurs when a property value changes
        EVT_PG_CHANGED( -1, CObjPropsCanvas::OnPropertyGridChange )
        EVT_PG_SELECTED( -1, CObjPropsCanvas::OnPropertyGridSelect )
END_EVENT_TABLE()

static void OnApply(wxCommandEvent& event)
{
	if(wxGetApp().m_frame->m_properties->OnApply2())
	{
		wxGetApp().m_marked_list->Clear(true);
	}
}

static void OnCancel(wxCommandEvent& event)
{
	wxGetApp().m_frame->m_properties->OnCancel2();
}

CObjPropsCanvas::CObjPropsCanvas(wxWindow* parent)
        : CPropertiesCanvas(parent)
{
	m_toolBar = NULL;
	m_make_initial_properties_in_refresh = false;
	m_object_canvas = NULL;
	AddToolBar();
}

void CObjPropsCanvas::AddToolBar()
{
	if(m_toolBar)delete m_toolBar;

	// make a toolbar for the current input modes's tools
	m_toolBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_toolBar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));
	m_toolBar->Realize();
}

CObjPropsCanvas::~CObjPropsCanvas()
{
	ClearInitialProperties();
}

void CObjPropsCanvas::OnSize(wxSizeEvent& event)
{
	wxScrolledWindow::OnSize(event);

	Resize();

    event.Skip();
}

void CObjPropsCanvas::OnPropertyGridChange( wxPropertyGridEvent& event ) {
	CPropertiesCanvas::OnPropertyGridChange(event);
}

void CObjPropsCanvas::OnPropertyGridSelect( wxPropertyGridEvent& event ) {
	CPropertiesCanvas::OnPropertyGridSelect(event);
}

void CObjPropsCanvas::ClearInitialProperties()
{
	for(std::list<Property *>::iterator It = m_initial_properties.begin(); It != m_initial_properties.end(); It++)
	{
		Property* p = *It;
		delete p;
	}

	m_initial_properties.clear();
}

void CObjPropsCanvas::RefreshByRemovingAndAddingAll2(){
	ClearProperties();
	wxGetApp().m_frame->ClearToolBar(m_toolBar);

	HeeksObj* marked_object = NULL;
	if(wxGetApp().m_marked_list->size() == 1)
	{
		marked_object = (*wxGetApp().m_marked_list->list().begin());
	}

	if(m_make_initial_properties_in_refresh)ClearInitialProperties();

	ObjectCanvas* object_canvas = NULL;
	if(wxGetApp().m_marked_list->size() == 1)
		object_canvas = wxGetApp().m_marked_list->list().front()->GetDialog(this);
	if(m_object_canvas && object_canvas != m_object_canvas)
		m_object_canvas->Show(false); // hide the previous object canvas

	m_object_canvas = object_canvas;

	if(m_object_canvas)
		m_object_canvas->Show(); // show the next object canvas

	if(wxGetApp().m_marked_list->size() > 0)
	{

		if(m_object_canvas)
		{
			m_object_canvas->SetWithObject(wxGetApp().m_marked_list->list().front());
		}

		// use the property list too
		std::list<Property *> list;
		wxGetApp().m_marked_list->GetProperties(&list);
		for(std::list<Property*>::iterator It = list.begin(); It != list.end(); It++)
		{
			Property* property = *It;
			if(m_make_initial_properties_in_refresh)m_initial_properties.push_back(property->MakeACopy());
			AddProperty(property);
		}

		// add toolbar buttons
		wxString exe_folder = wxGetApp().GetExeFolder();
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, _("Apply"), wxBitmap(ToolImage(_T("apply"))), _("Apply any changes made to the properties"), OnApply);
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, _("Cancel"), wxBitmap(ToolImage(_T("cancel"))), _("Stop editing the object"), OnCancel);

		std::list<Tool*> t_list;
		MarkedObjectOneOfEach mo(0, marked_object, 1, 0, NULL);
		wxGetApp().m_marked_list->GetTools(&mo, t_list, NULL, false);
		for(std::list<Tool*>::iterator It = t_list.begin(); It != t_list.end(); It++)
		{
			Tool* tool = *It;
			if(tool)wxGetApp().m_frame->AddToolBarTool(m_toolBar, tool);
		}

		m_toolBar->Realize();
	}

	Resize();
}

void CObjPropsCanvas::Resize()
{
	// resize property grid and toolbar

	// change size for toolbar
	wxSize size = GetClientSize();
	wxSize pg_size = size;

	if(m_toolBar->GetToolsCount() > 0){
		int toolbar_height = ToolImage::GetBitmapSize() + EXTRA_TOOLBAR_HEIGHT;
		pg_size = wxSize(size.x, size.y - toolbar_height);
		m_toolBar->SetSize(0, pg_size.y , size.x, toolbar_height );
		m_toolBar->Show();
	}
	else
	{
		m_toolBar->Show(false);
	}

	if(m_object_canvas)
	{
		if(pmap.size() > 0)
		{
			wxSize half_size = wxSize(pg_size.x, pg_size.y / 2);
			pg_size = wxSize(size.x, pg_size.y - half_size.y);
			m_object_canvas->SetSize(0, pg_size.y, size.x, half_size.y);
		}
		else
			m_object_canvas->SetSize(0, 0, pg_size.x, pg_size.y);
	}

	// change size for property grid
	m_pg->SetSize(0, 0, pg_size.x, pg_size.y);
}

bool CObjPropsCanvas::OnApply2()
{
	// cause all of the properties to be applied
	ClearProperties();

	if(wxGetApp().m_marked_list->size() == 1)
	{
		HeeksObj* marked_object = (*wxGetApp().m_marked_list->list().begin());
		if(!marked_object->ValidateProperties())return false;
	}

	if(wxGetApp().m_marked_list->size() == 1)
	{
		HeeksObj* marked_object = (*wxGetApp().m_marked_list->list().begin());
		marked_object->OnApplyProperties();
	}

	return true;
}

static bool in_OnCancel2 = false;

void CObjPropsCanvas::OnCancel2()
{
	in_OnCancel2 = true;
	ClearProperties();
	for(std::list<Property*>::iterator It = m_initial_properties.begin(); It != m_initial_properties.end(); It++){
		Property* p = *It;
		p->CallSetFunction();
	}
	wxGetApp().m_marked_list->Clear(true);
	in_OnCancel2 = false;
}

void CObjPropsCanvas::WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list)
{
	if(in_OnCancel2)return;

	m_make_initial_properties_in_refresh = true;
	RefreshByRemovingAndAddingAll();
	m_make_initial_properties_in_refresh = false;
}

void CObjPropsCanvas::OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)
{
	if(in_OnCancel2)return;

	RefreshByRemovingAndAddingAll();
}
