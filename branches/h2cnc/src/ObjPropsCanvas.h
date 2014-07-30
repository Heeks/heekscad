// ObjPropsCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "PropertiesCanvas.h"

class CObjPropsCanvas: public CPropertiesCanvas
{
private:
	wxToolBar *m_toolBar;
	std::list<Property *> m_initial_properties;
	bool m_make_initial_properties_in_refresh;
	ObjectCanvas* m_object_canvas;

	void ClearInitialProperties();

public:
    CObjPropsCanvas(wxWindow* parent);
    virtual ~CObjPropsCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );
    void OnPropertyGridSelect( wxPropertyGridEvent& event );

// Observer's virtual functions
    void WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list);
	void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);

	// CPropertiesCanvas's virtual functions
	void RefreshByRemovingAndAddingAll2();

	void AddToolBar();
	bool OnApply2();
	void OnCancel2();

	void Resize();

    DECLARE_NO_COPY_CLASS(CObjPropsCanvas)
    DECLARE_EVENT_TABLE()
};
