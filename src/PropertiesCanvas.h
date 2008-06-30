#pragma once

#include "Observer.h"

class HeeksObj;
class Property;
class wxPropertyGrid;
class wxPGProperty;
class wxPropertyGridEvent;

class CPropertiesCanvas: public wxScrolledWindow, public Observer
{
public:
    CPropertiesCanvas(wxWindow* parent, bool wants_apply_cancel_toolbar = true);
    virtual ~CPropertiesCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );
	void OnCancel2(wxCommandEvent& event);

private:
    std::map<wxPGProperty*, Property*> pmap;
    std::set<Property*> pset;
    wxPropertyGrid* m_pg;
	wxToolBar *m_toolBar;
	HeeksObj* m_object_for_cancel;
	HeeksObj* m_copy_for_cancel;

protected:
    void Resize();
    void Append(wxPGProperty* parent_prop, wxPGProperty* new_prop, Property* property);
    void ClearProperties();
    void AddProperty(Property* property, wxPGProperty* parent_prop = NULL);
    void RemoveProperty(Property* property);
    Property* GetProperty(wxPGProperty* property);

// Observer's virtual functions
    void WhenMarkedListChanges(bool all_added, bool all_removed, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list);

public:
	virtual void RefreshByRemovingAndAddingAll();
	void OnApply2();

    DECLARE_NO_COPY_CLASS(CPropertiesCanvas)
    DECLARE_EVENT_TABLE()
};
