#include "stdafx.h"
#include "wx/dcmirror.h"
#include "../interface/InputMode.h"
#include "InputModeCanvas.h"
#include "../interface/Property.h"
#include "propgrid.h"
#include "HeeksFrame.h"

BEGIN_EVENT_TABLE(CInputModeCanvas, wxScrolledWindow)
	EVT_SIZE(CInputModeCanvas::OnSize)

        // This occurs when a property value changes
        EVT_PG_CHANGED( -1, CInputModeCanvas::OnPropertyGridChange )
END_EVENT_TABLE()


CInputModeCanvas::CInputModeCanvas(wxWindow* parent)
        : CPropertiesCanvas(parent)
{
	// make a toolbar for the current input modes's tools
	m_toolBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_toolBar->SetToolBitmapSize(wxSize(32, 32));
	m_toolBar->Realize();
}

CInputModeCanvas::~CInputModeCanvas()
{

}

void CInputModeCanvas::OnSize(wxSizeEvent& event)
{
	wxScrolledWindow::OnSize(event);

	wxSize size = GetClientSize();
	if(m_toolBar->GetToolsCount() > 0){
		wxSize toolbar_size = m_toolBar->GetClientSize();
		m_pg->SetSize(0, 0, size.x, size.y - 39 );
		m_toolBar->SetSize(0, size.y - 39 , size.x, 39 );
		m_toolBar->Show();
	}
	else{
		m_pg->SetSize(0, 0, size.x, size.y );
		m_toolBar->Show(false);
	}

    event.Skip();
}

void CInputModeCanvas::OnPropertyGridChange( wxPropertyGridEvent& event ) {
	CPropertiesCanvas::OnPropertyGridChange(event);
}

void CInputModeCanvas::RefreshByRemovingAndAddingAll(){
	ClearProperties();
	wxGetApp().m_frame->ClearToolBar(m_toolBar);

	std::list<Property *> list;

	// add the input_mode mode's properties
	wxGetApp().input_mode_object->GetProperties(&list);

	// add the properties to the grid
	std::list<Property *>::iterator It;
	for(It = list.begin(); It != list.end(); It++)
	{
		Property* property = *It;
		AddProperty(property);
	}

	// add toolbar buttons
	std::list<Tool*> t_list;
	wxGetApp().input_mode_object->GetTools(&t_list, NULL);
	for(std::list<Tool*>::iterator It = t_list.begin(); It != t_list.end(); It++)
	{
		Tool* tool = *It;
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, tool);
	}

	m_toolBar->Realize();

	wxSize size = GetClientSize();
	if(m_toolBar->GetToolsCount() > 0){
		wxSize toolbar_size = m_toolBar->GetClientSize();
		m_pg->SetSize(0, 0, size.x, size.y - 39 );
		m_toolBar->SetSize(0, size.y - 39 , size.x, 39 );
		m_toolBar->Show();
	}
	else{
		m_pg->SetSize(0, 0, size.x, size.y );
		m_toolBar->Show(false);
	}
}