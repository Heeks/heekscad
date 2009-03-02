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

class CPropertiesCanvas: public wxScrolledWindow, public Observer
{
public:
    CPropertiesCanvas(wxWindow* parent);
    virtual ~CPropertiesCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );

protected:
    std::map<wxPGProperty*, Property*> pmap;
    std::set<Property*> pset;
    wxPropertyGrid* m_pg;

    void Resize();
    void Append(wxPGProperty* parent_prop, wxPGProperty* new_prop, Property* property);
    void ClearProperties();
    void AddProperty(Property* property, wxPGProperty* parent_prop = NULL);
    Property* GetProperty(wxPGProperty* property);

public:
	void DeselectProperties();

    DECLARE_NO_COPY_CLASS(CPropertiesCanvas)
    DECLARE_EVENT_TABLE()
};
