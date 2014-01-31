// CuboidDlg.h
// Copyright (c) 2014, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "../interface/HeeksObjDlg.h"

class XYZBoxes;

class CuboidDlg : public HeeksObjDlg
{
	XYZBoxes *m_xyzPos;
	XYZBoxes *m_xyzSize;

public:
    CuboidDlg(wxWindow *parent, HeeksObj* object, const wxString& title = wxString(_T("Cuboid")), bool top_level = true);

	// HeeksObjDlg virtual functions
	void GetDataRaw(HeeksObj* object);
	void SetFromDataRaw(HeeksObj* object);
	void SetPictureByWindow(wxWindow* w);
	void SetPicture(const wxString& name);
	
    DECLARE_EVENT_TABLE()
};
