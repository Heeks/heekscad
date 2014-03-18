// InputModeCanvas.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "InputModeCanvas.h"
#include "../interface/InputMode.h"
#include "../interface/Property.h"
#include "../interface/PropertyString.h"
#include "../interface/ToolImage.h"
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
	m_toolBar = NULL;
	AddToolBar();
}

void CInputModeCanvas::AddToolBar()
{
	if(m_toolBar)delete m_toolBar;

	// make a toolbar for the current input modes's tools
	m_toolBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
	m_previous_tools.clear();
	m_toolBar->SetToolBitmapSize(wxSize(ToolImage::GetBitmapSize(), ToolImage::GetBitmapSize()));
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

void CInputModeCanvas::OnPropertyGridChange( wxPropertyGridEvent& event ) {
	CPropertiesCanvas::OnPropertyGridChange(event);
}

void CInputModeCanvas::RefreshByRemovingAndAddingAll2(){
	ClearProperties();

	std::list<Property *> list;

	// add the input_mode mode's properties
	PropertyString* title = new PropertyString(_("Input Mode"), wxGetApp().input_mode_object->GetTitle(), NULL);
	if(wxGetApp().input_mode_object->TitleHighlighted())title->m_highlighted = true;
	list.push_back(title);
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

	// compare to previous_list
	bool tools_changed = false;
	if(t_list.size() != m_previous_tools.size())tools_changed = true;
	else
	{
		std::list<Tool*>::iterator TIt = t_list.begin();
		for(std::list<Tool*>::iterator It = m_previous_tools.begin(); It != m_previous_tools.end(); It++, TIt++){
			Tool* pt = *It;
			Tool* t = *TIt;
			if(t != pt)
			{
				tools_changed = true;
				break;
			}
		}
	}

	if(tools_changed)
	{
		// remake tool bar
		wxGetApp().m_frame->ClearToolBar(m_toolBar);
		for(std::list<Tool*>::iterator It = t_list.begin(); It != t_list.end(); It++)
		{
			Tool* tool = *It;
			wxGetApp().m_frame->AddToolBarTool(m_toolBar, tool);
		}

		m_toolBar->Realize();

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

		m_previous_tools = t_list;
	}

}

