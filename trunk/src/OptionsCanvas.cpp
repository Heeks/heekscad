#include "stdafx.h"
#include "wx/dcmirror.h"
#include "OptionsCanvas.h"
#include "../interface/MarkedObject.h"
#include "../interface/Property.h"
#include "PropertyVertex.h"
#include "propgrid.h"

BEGIN_EVENT_TABLE(COptionsCanvas, wxScrolledWindow)
	EVT_SIZE(COptionsCanvas::OnSize)

        // This occurs when a property value changes
        EVT_PG_CHANGED( -1, COptionsCanvas::OnPropertyGridChange )
END_EVENT_TABLE()


COptionsCanvas::COptionsCanvas(wxWindow* parent)
        : CPropertiesCanvas(parent, false)
{
}

COptionsCanvas::~COptionsCanvas()
{

}

void COptionsCanvas::OnSize(wxSizeEvent& event)
{
   CPropertiesCanvas::OnSize(event);

    event.Skip();
}

void COptionsCanvas::OnPropertyGridChange( wxPropertyGridEvent& event ) {
	CPropertiesCanvas::OnPropertyGridChange(event);
}

void COptionsCanvas::RefreshByRemovingAndAddingAll(){
	ClearProperties();

	std::list<Property *> list;

	// add the input_mode mode's properties
	wxGetApp().input_mode_object->GetProperties(&list);

	// add the application's properties
	wxGetApp().GetProperties(&list);

	// add the properties to the grid
	std::list<Property *>::iterator It;
	for(It = list.begin(); It != list.end(); It++)
	{
		Property* property = *It;
		AddProperty(property);
	}
}