// ObjPropsCanvas.cpp
#include "stdafx.h"
#include "wx/dcmirror.h"
#include "ObjPropsCanvas.h"
#include "../interface/Property.h"
#include "../interface/ToolImage.h"
#include "PropertyVertex.h"
#include "propgrid.h"
#include "HeeksFrame.h"
#include "MarkedList.h"
#include "../interface/MarkedObject.h"

BEGIN_EVENT_TABLE(CObjPropsCanvas, wxScrolledWindow)
	EVT_SIZE(CObjPropsCanvas::OnSize)

        // This occurs when a property value changes
        EVT_PG_CHANGED( -1, CObjPropsCanvas::OnPropertyGridChange )
END_EVENT_TABLE()

static void OnApply(wxCommandEvent& event)
{
	wxGetApp().m_frame->m_properties->OnApply2();
}

static void OnCancel(wxCommandEvent& event)
{
	// just deselect the object, and the cancel will happen automatically in RefreshByRemovingAndAddingAll
	wxGetApp().m_marked_list->Clear();
}

CObjPropsCanvas::CObjPropsCanvas(wxWindow* parent)
        : CPropertiesCanvas(parent), m_object_for_cancel(NULL)
{
	m_toolBar = NULL;
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

	wxSize size = GetClientSize();
	if(m_toolBar->GetToolsCount() > 0){
		wxSize toolbar_size = m_toolBar->GetClientSize();
		int toolbar_height = ToolImage::GetBitmapSize() + EXTRA_TOOLBAR_HEIGHT;
		m_pg->SetSize(0, 0, size.x, size.y - toolbar_height );
		m_toolBar->SetSize(0, size.y - toolbar_height , size.x, toolbar_height );
		m_toolBar->Show();
	}
	else{
		m_pg->SetSize(0, 0, size.x, size.y );
		m_toolBar->Show(false);
	}

    event.Skip();
}

void CObjPropsCanvas::OnPropertyGridChange( wxPropertyGridEvent& event ) {
	CPropertiesCanvas::OnPropertyGridChange(event);
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

void CObjPropsCanvas::RefreshByRemovingAndAddingAll(){
	ClearProperties();
	wxGetApp().m_frame->ClearToolBar(m_toolBar);

	HeeksObj* marked_object = NULL;
	if(wxGetApp().m_marked_list->size() == 1)
	{
		marked_object = (*wxGetApp().m_marked_list->list().begin());
	}

	if(marked_object != m_object_for_cancel)
	{
		// if an object becomes unselected, this cancels the editing
		for(std::list<Property*>::iterator It = m_initial_properties.begin(); It != m_initial_properties.end(); It++){
			Property* p = *It;
			p->CallSetFunction();
		}
	}

	ClearInitialProperties();

	m_object_for_cancel = marked_object;

	if(wxGetApp().m_marked_list->size() > 0)
	{
		std::list<Property *> list;
		wxGetApp().m_marked_list->GetProperties(&list);
		for(std::list<Property*>::iterator It = list.begin(); It != list.end(); It++)
		{
			Property* property = *It;
			m_initial_properties.push_back(property->MakeACopy());
			AddProperty(property);
		}

		// add toolbar buttons
		wxString exe_folder = wxGetApp().GetExeFolder();
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, _("Apply"), wxBitmap(ToolImage(_T("apply"))), _("Apply any changes made to the properties"), OnApply);
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, _("Cancel"), wxBitmap(ToolImage(_T("cancel"))), _("Stop editing the object"), OnCancel);

		std::list<Tool*> t_list;
		wxGetApp().m_marked_list->GetTools(&MarkedObjectOneOfEach(0, marked_object, 1), t_list, NULL);
		for(std::list<Tool*>::iterator It = t_list.begin(); It != t_list.end(); It++)
		{
			Tool* tool = *It;
			if(tool)wxGetApp().m_frame->AddToolBarTool(m_toolBar, tool);
		}

		m_toolBar->Realize();

		// resize property grid and toolbar
		wxSize size = GetClientSize();
		if(m_toolBar->GetToolsCount() > 0){
			wxSize toolbar_size = m_toolBar->GetClientSize();
			int toolbar_height = ToolImage::GetBitmapSize() + EXTRA_TOOLBAR_HEIGHT;
			m_pg->SetSize(0, 0, size.x, size.y - toolbar_height );
			m_toolBar->SetSize(0, size.y - toolbar_height , size.x, toolbar_height );
			m_toolBar->Show();
		}
		else{
			m_pg->SetSize(0, 0, size.x, size.y );
			m_toolBar->Show(false);
		}
	}
}

void CObjPropsCanvas::ApplyChanges()
{
	m_object_for_cancel = NULL;
}

void CObjPropsCanvas::OnApply2()
{
	// cause all of the properties to be applied
	ClearProperties();

	if(wxGetApp().m_marked_list->size() == 1)
	{
		HeeksObj* marked_object = (*wxGetApp().m_marked_list->list().begin());
		marked_object->OnApplyProperties();
	}

	ApplyChanges();
}

void CObjPropsCanvas::WhenMarkedListChanges(bool all_added, bool all_removed, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list)
{
	RefreshByRemovingAndAddingAll();
}

void CObjPropsCanvas::OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)
{
}
