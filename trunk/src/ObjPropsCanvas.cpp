// ObjPropsCanvas.cpp
#include "stdafx.h"
#include "wx/dcmirror.h"
#include "ObjPropsCanvas.h"
#include "../interface/Property.h"
#include "PropertyVertex.h"
#include "propgrid.h"
#include "HeeksFrame.h"
#include "MarkedList.h"

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
        : CPropertiesCanvas(parent), m_copy_for_cancel(NULL)
{
	// make a tool bar with an "Apply" tick and a "Cancel" cross.
	m_toolBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_toolBar->SetToolBitmapSize(wxSize(32, 32));
	m_toolBar->Realize();
}

CObjPropsCanvas::~CObjPropsCanvas()
{
	if(m_copy_for_cancel)
	{
		delete m_copy_for_cancel;
		m_copy_for_cancel = NULL;
	}
}

void CObjPropsCanvas::OnSize(wxSizeEvent& event)
{
	wxScrolledWindow::OnSize(event);

	wxSize size = GetClientSize();
	wxSize toolbar_size = m_toolBar->GetClientSize();
	m_pg->SetSize(0, 0, size.x, size.y - toolbar_size.y );
	m_toolBar->SetSize(0, size.y - toolbar_size.y , size.x, toolbar_size.y );

    event.Skip();
}

void CObjPropsCanvas::OnPropertyGridChange( wxPropertyGridEvent& event ) {
	CPropertiesCanvas::OnPropertyGridChange(event);
}

void CObjPropsCanvas::RefreshByRemovingAndAddingAll(){
	ClearProperties();

	if(m_copy_for_cancel)
	{
		// if an object becomes unselected, this cancels the editing
		m_object_for_cancel->CopyFrom(m_copy_for_cancel);
		wxGetApp().WasModified(m_object_for_cancel);
		delete m_copy_for_cancel;
		m_copy_for_cancel = NULL;
	}

	wxGetApp().m_frame->ClearToolBar(m_toolBar);

	if(wxGetApp().m_marked_list->size() == 1)
	{
		HeeksObj* object = (*wxGetApp().m_marked_list->list().begin());
		m_object_for_cancel = object;
		m_copy_for_cancel = object->MakeACopy();

		std::list<Property *> list;
		object->GetProperties(&list);
		for(std::list<Property*>::iterator It = list.begin(); It != list.end(); It++)
		{
			Property* property = *It;
			AddProperty(property);
		}

		// add toolbar buttons
		wxString exe_folder = wxGetApp().GetExeFolder();
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, _T("Apply"), wxBitmap(exe_folder + "/bitmaps/apply.png", wxBITMAP_TYPE_PNG), _T("Apply any changes made to the properties"), OnApply);
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, _T("Cancel"), wxBitmap(exe_folder + "/bitmaps/cancel.png", wxBITMAP_TYPE_PNG), _T("Stop editing the object"), OnCancel);

		std::list<Tool*> t_list;
		object->GetTools(&t_list, NULL);
		for(std::list<Tool*>::iterator It = t_list.begin(); It != t_list.end(); It++)
		{
			Tool* tool = *It;
			wxGetApp().m_frame->AddToolBarTool(m_toolBar, tool);
		}

		m_toolBar->Realize();

		// resize property grid and toolbar
		wxSize size = GetClientSize();
		wxSize toolbar_size = m_toolBar->GetClientSize();
		m_pg->SetSize(0, 0, size.x, size.y - toolbar_size.y );
		m_toolBar->SetSize(0, size.y - toolbar_size.y , size.x, toolbar_size.y );
	}
}

void CObjPropsCanvas::OnApply2()
{
	if(m_copy_for_cancel)
	{
		delete m_copy_for_cancel;
		m_copy_for_cancel = NULL;
	}

	// cause all of the properties to be applied
	RefreshByRemovingAndAddingAll();
}