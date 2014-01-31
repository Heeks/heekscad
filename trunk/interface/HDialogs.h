// HDialogs.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// this defines the base class for dialogs used in HeeksCAD

#pragma once

class HControl
{
public:
	wxWindow *m_w;
	wxSizer *m_s;
	int m_add_flag; // flag for wxBoxSizer.Add( , ,flag)

	HControl(wxWindow* w, int flag):m_w(w), m_s(NULL), m_add_flag(flag){}
	HControl(wxSizer* s, int flag):m_w(NULL), m_s(s), m_add_flag(flag){}

	wxSizerItem* AddToSizer(wxSizer* s);
};

class HTypeObjectDropDown: public wxComboBox
{
public:
	std::vector< std::pair< int, wxString > > ids_for_combo;
	HTypeObjectDropDown(wxWindow *parent, wxWindowID id, int object_type, HeeksObj* obj_list);
	int GetSelectedId();
	void SelectById(int id);

	static wxArrayString GetObjectArrayString(int object_type, HeeksObj* obj_list, std::vector< std::pair< int, wxString > > &ids_for_combo);
};

class CLengthCtrl;
class HDialog;

class XYZBoxes: public wxStaticBoxSizer
{
public:
	CLengthCtrl *m_lgthX;
	CLengthCtrl *m_lgthY;
	CLengthCtrl *m_lgthZ;
	wxStaticText *m_sttcX;
	wxStaticText *m_sttcY;
	wxStaticText *m_sttcZ;

	XYZBoxes(HDialog *dlg, const wxString& label, const wxString &xstr, const wxString &ystr, const wxString &zstr);
};

class HDialog : public wxDialog
{
public:
	static const int control_border; // the border around controls

	bool m_ignore_event_functions;

	HControl MakeLabelAndControl(const wxString& label, wxWindow* control, wxStaticText** static_text = NULL);
	wxStaticText *AddLabelAndControl(wxBoxSizer* sizer, const wxString& label, wxWindow* control);
	HControl MakeOkAndCancel(int orient);

	HDialog(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& title = wxString(_T("")), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxDialogNameStr);
};
