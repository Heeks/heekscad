// HeeksObjDlg.h
// Copyright (c) 2014, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class HeeksObj;
class PictureWindow;

#include "HDialogs.h"

class HeeksObjDlg : public HDialog
{
	PictureWindow *m_picture;

protected:
	HeeksObj* m_object;
	std::list<HControl> leftControls;
	std::list<HControl> rightControls;

public:
	HeeksObjDlg(wxWindow *parent, HeeksObj* object, const wxString& title = wxString(_T("")), bool top_level = true, bool picture = true);

	virtual void GetDataRaw(HeeksObj* object);
	virtual void SetFromDataRaw(HeeksObj* object);
	virtual void SetPictureByWindow(wxWindow* w);
	virtual void SetPicture(const wxString& name);
	
	void GetData(HeeksObj* object);
	void SetFromData(HeeksObj* object);
	void SetPicture();
	void SetPicture(const wxString& name, const wxString& folder);
	void AddControlsAndCreate();

	void OnChildFocus(wxChildFocusEvent& event);
	void OnComboOrCheck( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};
