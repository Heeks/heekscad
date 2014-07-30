// HDialogs.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// this defines the base class for dialogs used in HeeksCAD

#pragma once

class HDialog : public wxDialog
{
public:
	static const int control_border; // the border around controls

	bool m_ignore_event_functions;

	void AddLabelAndControl(wxBoxSizer* sizer, const wxString& label, wxWindow* control);
	wxBoxSizer *MakeOkAndCancel(int orient);

	HDialog(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& title = wxString(_T("")), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxDialogNameStr);
};
