#include "stdafx.h"
#include "PropertiesCanvas.h"
#include "../interface/MarkedObject.h"
#include "../interface/Property.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyString.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyColor.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyList.h"
#include "PropertyVertex.h"
#include "propgrid.h"
#include "advprops.h"
#include "MarkedList.h"
#include "HeeksFrame.h"

BEGIN_EVENT_TABLE(CPropertiesCanvas, wxScrolledWindow)
	EVT_SIZE(CPropertiesCanvas::OnSize)

        // This occurs when a property value changes
        EVT_PG_CHANGED( -1, CPropertiesCanvas::OnPropertyGridChange )
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


CPropertiesCanvas::CPropertiesCanvas(wxWindow* parent, bool wants_apply_cancel_toolbar)
: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
				   wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE), m_copy_for_cancel(NULL)
{
	// Assumes code is in frame/dialog constructor

	// Construct wxPropertyGrid control
	m_pg = new wxPropertyGrid(
		this, // parent
		-1, // id
		wxDefaultPosition, // position
		wxDefaultSize, // size
		// Some specific window styles - for all additional styles,
		// see Modules->PropertyGrid Window Styles
		wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
		// Default style
		wxPG_DEFAULT_STYLE | wxBORDER_THEME );

	m_pg->SetExtraStyle( wxPG_EX_HELP_AS_TOOLTIPS );  

	if(wants_apply_cancel_toolbar)
	{
		// make a tool bar with an "Apply" tick and a "Cancel" cross.
		m_toolBar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT);
		m_toolBar->SetToolBitmapSize(wxSize(32, 32));

		wxString exe_folder = wxGetApp().GetExeFolder();

		wxImage::AddHandler(new wxPNGHandler);
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, _T("Apply"), wxBitmap(exe_folder + "/bitmaps/apply.png", wxBITMAP_TYPE_PNG), _T("Apply any changes made to the properties"), OnApply);
		wxGetApp().m_frame->AddToolBarTool(m_toolBar, _T("Cancel"), wxBitmap(exe_folder + "/bitmaps/cancel.png", wxBITMAP_TYPE_PNG), _T("Stop editing the object"), OnCancel);

		m_toolBar->Realize();
	}
	else
	{
		m_toolBar = NULL;
	}

	wxGetApp().RegisterObserver(this);
}

CPropertiesCanvas::~CPropertiesCanvas()
{
	if(m_copy_for_cancel)
	{
		delete m_copy_for_cancel;
		m_copy_for_cancel = NULL;
	}
	ClearProperties();
}

void CPropertiesCanvas::OnSize(wxSizeEvent& event)
{
	wxScrolledWindow::OnSize(event);

	wxSize size = GetClientSize();

	if(m_toolBar)
	{
		wxSize toolbar_size = m_toolBar->GetClientSize();
		m_pg->SetSize(0, 0, size.x, size.y - toolbar_size.y );
		m_toolBar->SetSize(0, size.y - toolbar_size.y , size.x, toolbar_size.y );
	}
	else
	{
		m_pg->SetSize(0, 0, size.x, size.y );
	}

    event.Skip();
}

void CPropertiesCanvas::ClearProperties()
{
	m_pg->Clear();
	pmap.clear();

	{
	std::set<Property*>::iterator It;
	for(It = pset.begin(); It != pset.end(); It++)
	{
		Property* p = *It;
		delete p;
	}
	}
	pset.clear();
}

void CPropertiesCanvas::Append(wxPGProperty* parent_prop, wxPGProperty* new_prop, Property* property)
{
	if(parent_prop){
		m_pg->AppendIn(parent_prop, new_prop);
	}
	else
	{
		m_pg->Append(new_prop);
		pset.insert(property);
	}
	pmap.insert(std::pair<wxPGProperty*, Property*>( new_prop, property));
}

void CPropertiesCanvas::AddProperty(Property* p, wxPGProperty* parent_prop)
{
	switch(p->get_property_type()){
	case StringPropertyType:
		{
			wxPGProperty *new_prop = wxStringProperty(p->GetShortString(),wxPG_LABEL, ((PropertyString*)p)->m_initial_value);
			if(!p->property_editable())new_prop->SetFlag(wxPG_PROP_READONLY);
			Append( parent_prop, new_prop, p);
		}
		break;
	case DoublePropertyType:
		{
			wxPGProperty *new_prop = wxFloatProperty(p->GetShortString(),wxPG_LABEL, ((PropertyDouble*)p)->m_initial_value);
			if(!p->property_editable())new_prop->SetFlag(wxPG_PROP_READONLY);
			Append( parent_prop, new_prop, p);
		}
		break;
	case IntPropertyType:
		{
			wxPGProperty *new_prop = wxIntProperty(p->GetShortString(),wxPG_LABEL, ((PropertyInt*)p)->m_initial_value);
			if(!p->property_editable())new_prop->SetFlag(wxPG_PROP_READONLY);
			Append( parent_prop, new_prop, p);
		}
		break;
	case ColorPropertyType:
		{
			HeeksColor& col = ((PropertyColor*)p)->m_initial_value;
			wxColour wcol(col.red, col.green, col.blue);
			wxPGProperty *new_prop = wxColourProperty(p->GetShortString(),wxPG_LABEL, wcol);
			if(!p->property_editable())new_prop->SetFlag(wxPG_PROP_READONLY);
			Append( parent_prop, new_prop, p);
		}
		break;
	case ChoicePropertyType:
		{
			wxArrayString array_string;
			std::list< std::string >::iterator It;
			for(It = ((PropertyChoice*)p)->m_choices.begin(); It != ((PropertyChoice*)p)->m_choices.end(); It++){
				array_string.Add(wxString(It->c_str()));
			}
			wxPGProperty *new_prop = wxEnumProperty(p->GetShortString(),wxPG_LABEL,array_string, ((PropertyChoice*)p)->m_initial_index);
			if(!p->property_editable())new_prop->SetFlag(wxPG_PROP_READONLY);
			Append( parent_prop, new_prop, p );
		}
		break;
	case VertexPropertyType:
		{
			double x[3];
			extract(((PropertyVertex*)p)->m_vt, x);
			wxPGProperty* new_prop = wxParentProperty(p->GetShortString(),wxPG_LABEL);
			Append( parent_prop, new_prop, p );
			wxPGProperty* x_prop = wxFloatProperty("x",wxPG_LABEL,x[0]);
			if(!p->property_editable())x_prop->SetFlag(wxPG_PROP_READONLY);
			Append( new_prop, x_prop, p );
			wxPGProperty* y_prop = wxFloatProperty("y",wxPG_LABEL,x[1]);
			if(!p->property_editable())y_prop->SetFlag(wxPG_PROP_READONLY);
			Append( new_prop, y_prop, p );
			wxPGProperty* z_prop = wxFloatProperty("z",wxPG_LABEL,x[2]);
			if(!p->property_editable())z_prop->SetFlag(wxPG_PROP_READONLY);
			new_prop->SetFlag(wxPG_PROP_READONLY);
			Append( new_prop, z_prop, p );
		}
		break;
	case CheckPropertyType:
		{
			wxPGProperty* new_prop = wxBoolProperty(p->GetShortString(),wxPG_LABEL, ((PropertyCheck*)p)->m_initial_value);
			if(!p->property_editable())new_prop->SetFlag(wxPG_PROP_READONLY);
			Append( parent_prop, new_prop, p );
			m_pg->SetPropertyAttribute(new_prop, wxPG_BOOL_USE_CHECKBOX, true);
		}
		break;
	case ListOfPropertyType:
		{
			wxPGProperty* new_prop = wxParentProperty(p->GetShortString(),wxPG_LABEL);
			if(!p->property_editable())new_prop->SetFlag(wxPG_PROP_READONLY);
			Append( parent_prop, new_prop, p );
			std::list< Property* >::iterator It;
			for(It = ((PropertyList*)p)->m_list.begin(); It != ((PropertyList*)p)->m_list.end(); It++){
				Property* p2 = *It;
				AddProperty(p2, new_prop);
			}
		}
		break;
	}
}

void CPropertiesCanvas::RemoveProperty(Property* p)
{
	// to do, if needed
}

Property* CPropertiesCanvas::GetProperty(wxPGProperty* p)
{
	std::map<wxPGProperty*, Property*>::iterator FindIt;
	FindIt = pmap.find(p);
	if(FindIt == pmap.end())return NULL;
	return FindIt->second;
}

void CPropertiesCanvas::OnPropertyGridChange( wxPropertyGridEvent& event ) {
	wxPGProperty* p = event.GetPropertyPtr();

	Property* property = GetProperty(p);
	if(property == NULL)return;

	switch(property->get_property_type()){
	case StringPropertyType:
		{
			(*(((PropertyString*)property)->m_callbackfunc))(event.GetPropertyValue().GetString());
		}
		break;
	case DoublePropertyType:
		{
			(*(((PropertyDouble*)property)->m_callbackfunc))(event.GetPropertyValue().GetDouble());
		}
		break;
	case IntPropertyType:
		{
			(*(((PropertyInt*)property)->m_callbackfunc))(event.GetPropertyValue().GetLong());
		}
		break;
	case ColorPropertyType:
		{
			wxVariant var = event.GetPropertyValue();
			const wxColour* wcol = wxGetVariantCast(var,wxColour);
			HeeksColor col(wcol->Red(), wcol->Green(), wcol->Blue());
			(*(((PropertyColor*)property)->m_callbackfunc))(col);
		}
		break;
	case VertexPropertyType:
		{
			double pos[3];
			extract(((PropertyVertex*)property)->m_vt, pos);
			if(p->GetName()[0] == 'x'){
				pos[0] = event.GetPropertyValue().GetDouble();
			}
			else if(p->GetName()[0] == 'y'){
				pos[1] = event.GetPropertyValue().GetDouble();
			}
			else if(p->GetName()[0] == 'z'){
				pos[2] = event.GetPropertyValue().GetDouble();
			}
			(*(((PropertyVertex*)property)->m_callbackfunc))(make_point(pos));
		}
		break;
	case ChoicePropertyType:
		{
			(*(((PropertyChoice*)property)->m_callbackfunc))(event.GetPropertyValue().GetLong());
		}
		break;
	case CheckPropertyType:
		{
			(*(((PropertyCheck*)property)->m_callbackfunc))(event.GetPropertyValue().GetBool());
		}
		break;
	case ListOfPropertyType:
		{
		}
		break;
	}
}

void CPropertiesCanvas::WhenMarkedListChanges(bool all_added, bool all_removed, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list)
{
	RefreshByRemovingAndAddingAll();
}

void CPropertiesCanvas::RefreshByRemovingAndAddingAll()
{
	ClearProperties();

	if(m_copy_for_cancel)
	{
		// if an object becomes unselected, this cancels the editing
		m_object_for_cancel->CopyFrom(m_copy_for_cancel);
		wxGetApp().WasModified(m_object_for_cancel);
		delete m_copy_for_cancel;
		m_copy_for_cancel = NULL;
	}

	if(wxGetApp().m_marked_list->size() == 1)
	{
		HeeksObj* object = (*wxGetApp().m_marked_list->list().begin());
		m_object_for_cancel = object;
		m_copy_for_cancel = object->MakeACopy();

		std::list<Property *> list;
		object->GetProperties(&list);
		std::list<Property*>::iterator It;
		for(It = list.begin(); It != list.end(); It++)
		{
			Property* property = *It;
			AddProperty(property);
		}
	}
}

void CPropertiesCanvas::OnApply2()
{
	if(m_copy_for_cancel)
	{
		delete m_copy_for_cancel;
		m_copy_for_cancel = NULL;
	}

	// cause all of the properties to be applied
	RefreshByRemovingAndAddingAll();
}