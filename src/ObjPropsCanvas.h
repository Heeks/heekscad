// ObjPropsCanvas.h

#pragma once

#include "PropertiesCanvas.h"

class CObjPropsCanvas: public CPropertiesCanvas
{
private:
	wxToolBar *m_toolBar;
	HeeksObj* m_object_for_cancel;
	HeeksObj* m_copy_for_cancel;

public:
    CObjPropsCanvas(wxWindow* parent);
    virtual ~CObjPropsCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );

// Observer's virtual functions
    void WhenMarkedListChanges(bool all_added, bool all_removed, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list);
	void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);

	// CPropertiesCanvas's virtual functions
	void RefreshByRemovingAndAddingAll();
	void OnApply2();

    DECLARE_NO_COPY_CLASS(CObjPropsCanvas)
    DECLARE_EVENT_TABLE()
};
