// HDialogs.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <stdafx.h>

#include "HDialogs.h"

const int HDialog::control_border = 3;

HDialog::HDialog(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	:wxDialog(parent, id, title, pos, size, style, name), m_ignore_event_functions(false)
{
}

void HDialog::AddLabelAndControl(wxBoxSizer* sizer, const wxString& label, wxWindow* control)
{
    wxBoxSizer *sizer_horizontal = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *static_label = new wxStaticText(this, wxID_ANY, label);
	sizer_horizontal->Add( static_label, 0, wxRIGHT | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, control_border );
	sizer_horizontal->Add( control, 1, wxLEFT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, control_border );
	sizer->Add( sizer_horizontal, 0, wxEXPAND | wxALL, control_border );
}

wxBoxSizer *HDialog::MakeOkAndCancel(int orient)
{
    wxBoxSizer *sizerOKCancel = new wxBoxSizer(orient);
    wxButton* buttonOK = new wxButton(this, wxID_OK, _("OK"));
	sizerOKCancel->Add( buttonOK, 0, wxALL, control_border );
    wxButton* buttonCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
	sizerOKCancel->Add( buttonCancel, 0, wxALL, control_border );
    buttonOK->SetDefault();
	return sizerOKCancel;
}
