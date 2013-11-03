// PropertiesCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Observer.h"

class HeeksObj;
class Property;
class wxPropertyGrid;
class wxPGProperty;
class wxPropertyGridEvent;

#ifdef WIN32
#define EXTRA_TOOLBAR_HEIGHT 7
#else
#define EXTRA_TOOLBAR_HEIGHT 14
#endif

class PropertyChange: public Tool {
public:
	const wxChar* GetTitle(){return _("Change Property");}
	void Run();
	void RollBack();
	bool Undoable(){return true;}
};

class PropertyMapItem
{
public:
	wxPGProperty* m_prop;// NULL if top level
	std::map<wxString, PropertyMapItem> m_children;
	std::list<Property*> m_properties;

	PropertyMapItem(wxPGProperty* prop){m_prop = prop;}

	PropertyMapItem* OnAddProperty(wxPGProperty* prop, bool& new_item);
	PropertyMapItem* FindItem(const wxString& str);
};

class CPropertiesCanvas: public wxScrolledWindow, public Observer
{
	bool m_frozen;
	bool m_refresh_wanted_on_thaw;

public:
    CPropertiesCanvas(wxWindow* parent);
    virtual ~CPropertiesCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );
    void OnPropertyGridSelect( wxPropertyGridEvent& event );

	// Observer's virtual functions
	void Freeze();
	void Thaw();

protected:
    wxPropertyGrid* m_pg;
	PropertyMapItem* m_map;
	std::map<wxPGProperty*, PropertyMapItem* > pmap;
	std::set<Property*> pset;

	PropertyMapItem* FindMapItem(wxPGProperty* property);

    void Resize();
    void Append(wxPGProperty* parent_prop, wxPGProperty* new_prop, Property* property);
    void ClearProperties();
    void AddProperty(Property* property, wxPGProperty* parent_prop = NULL);
	std::list<Property*>* GetProperties(wxPGProperty* property);

public:
	void DeselectProperties();
	void RefreshByRemovingAndAddingAll();
	virtual void RefreshByRemovingAndAddingAll2(){}

    DECLARE_NO_COPY_CLASS(CPropertiesCanvas)
    DECLARE_EVENT_TABLE()
};
