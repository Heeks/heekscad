// OptionsCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "PropertiesCanvas.h"

class COptionsCanvas: public CPropertiesCanvas
{
public:
    COptionsCanvas(wxWindow* parent);
    virtual ~COptionsCanvas();

    //virtual void OnDraw(wxDC& dc);
    void OnSize(wxSizeEvent& event);
    void OnPropertyGridChange( wxPropertyGridEvent& event );

public:
	void RefreshByRemovingAndAddingAll2();

    DECLARE_NO_COPY_CLASS(COptionsCanvas)
    DECLARE_EVENT_TABLE()
};
