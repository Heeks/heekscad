// OptionsCanvas.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "OptionsCanvas.h"
#include "../interface/MarkedObject.h"
#include "../interface/Property.h"
#include "../interface/InputMode.h"
#include "../interface/PropertyVertex.h"
#include "propgrid.h"
#include "HeeksFrame.h"

BEGIN_EVENT_TABLE(COptionsCanvas, wxScrolledWindow)
	EVT_SIZE(COptionsCanvas::OnSize)

        // This occurs when a property value changes
        EVT_PG_CHANGED( -1, COptionsCanvas::OnPropertyGridChange )
END_EVENT_TABLE()


COptionsCanvas::COptionsCanvas(wxWindow* parent)
        : CPropertiesCanvas(parent)
{
}

COptionsCanvas::~COptionsCanvas()
{

}

void COptionsCanvas::OnSize(wxSizeEvent& event)
{
	wxScrolledWindow::OnSize(event);

	wxSize size = GetClientSize();
	m_pg->SetSize(0, 0, size.x, size.y );

    event.Skip();
}

void COptionsCanvas::OnPropertyGridChange( wxPropertyGridEvent& event ) {
	CPropertiesCanvas::OnPropertyGridChange(event);
}

void COptionsCanvas::RefreshByRemovingAndAddingAll2(){
	ClearProperties();

	std::list<Property *> list;

	// add the application's properties
	wxGetApp().GetOptions(&list);

	// add the properties to the grid
	std::list<Property *>::iterator It;
	for(It = list.begin(); It != list.end(); It++)
	{
		Property* property = *It;
		AddProperty(property);
	}
}

