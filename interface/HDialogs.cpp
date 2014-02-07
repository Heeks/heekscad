// HDialogs.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <stdafx.h>
#ifdef HEEKSCNC
#include <CTool.h>
#endif

#include "HDialogs.h"
#include "NiceTextCtrl.h"

wxSizerItem* HControl::AddToSizer(wxSizer* s)
{
	if(m_w != NULL)return s->Add( m_w, 0, m_add_flag, HDialog::control_border );
	else return s->Add( m_s, 0, m_add_flag, HDialog::control_border );
}

std::vector< std::pair< int, wxString > > global_ids_for_combo;

HTypeObjectDropDown::HTypeObjectDropDown(wxWindow *parent, wxWindowID id, int object_type, HeeksObj* obj_list)
:m_ids_for_combo(global_ids_for_combo), m_object_type(object_type), m_obj_list(obj_list)
,wxComboBox(parent, id, _T(""), wxDefaultPosition, wxDefaultSize, GetObjectArrayString(object_type, obj_list, global_ids_for_combo))
{
}

void HTypeObjectDropDown::Recreate()
{
	Clear();
	Append(GetObjectArrayString(m_object_type, m_obj_list, m_ids_for_combo));
	global_ids_for_combo.clear();
}

wxArrayString HTypeObjectDropDown::GetObjectArrayString(int object_type, HeeksObj* obj_list, std::vector< std::pair< int, wxString > > &ids_for_combo)
{
	ids_for_combo.clear();
	wxArrayString str_array;

	// Always add a value of zero to allow for an absense of object use.
	ids_for_combo.push_back( std::make_pair(0, _("None") ) );
	str_array.Add(_("None"));

	for(HeeksObj* ob = obj_list->GetFirstChild(); ob; ob = obj_list->GetNextChild())
	{
		if (ob->GetType() != object_type) continue;

		int number = ob->GetID();
#ifdef HEEKSCNC
		if(object_type == ToolType)number = ((CTool*)ob)->m_tool_number;
#endif
		ids_for_combo.push_back( std::make_pair( number, ob->GetShortString() ) );
		str_array.Add(ob->GetShortString());
	} // End for

	return str_array;
}

int HTypeObjectDropDown::GetSelectedId()
{
	if(GetSelection() < 0)return 0;
	return m_ids_for_combo[GetSelection()].first;
}

void HTypeObjectDropDown::SelectById(int id)
{
	// set the combo to the correct item
	for(unsigned int i = 0; i < m_ids_for_combo.size(); i++)
	{
		if(m_ids_for_combo[i].first == id)
		{
			SetSelection(i);
			break;
		}
	}
}

const int HDialog::control_border = 3;

HDialog::HDialog(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	:wxDialog(parent, id, title, pos, size, style, name), m_ignore_event_functions(false)
{
}

wxStaticText *HDialog::AddLabelAndControl(wxBoxSizer* sizer, const wxString& label, wxWindow* control)
{
	wxStaticText *static_label;
	HControl c = MakeLabelAndControl(label, control, &static_label);
	if(c.m_w)sizer->Add( c.m_w, 0, wxEXPAND | wxALL, control_border );
	else sizer->Add( c.m_s, 0, wxEXPAND | wxALL, control_border );
	return static_label;
}

HControl HDialog::MakeLabelAndControl(const wxString& label, wxWindow* control, wxStaticText** static_text)
{
    wxBoxSizer *sizer_horizontal = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *static_label = new wxStaticText(this, wxID_ANY, label);
	sizer_horizontal->Add( static_label, 0, wxRIGHT | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, control_border );
	sizer_horizontal->Add( control, 1, wxLEFT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, control_border );
	if(static_text)*static_text = static_label;

	return HControl(sizer_horizontal, wxEXPAND | wxALL);
}

HControl HDialog::MakeLabelAndControl(const wxString& label, wxWindow* control1, wxWindow* control2, wxStaticText** static_text)
{
    wxBoxSizer *sizer_horizontal = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *static_label = new wxStaticText(this, wxID_ANY, label);
	sizer_horizontal->Add( static_label, 0, wxRIGHT | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, control_border );
	sizer_horizontal->Add( control1, 1, wxLEFT | wxRIGHT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, control_border );
	sizer_horizontal->Add( control2, 0, wxLEFT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, control_border );
	if(static_text)*static_text = static_label;

	return HControl(sizer_horizontal, wxEXPAND | wxALL);
}

HControl HDialog::MakeOkAndCancel(int orient)
{
    wxBoxSizer *sizerOKCancel = new wxBoxSizer(orient);
    wxButton* buttonOK = new wxButton(this, wxID_OK, _("OK"));
	sizerOKCancel->Add( buttonOK, 0, wxALL, control_border );
    wxButton* buttonCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
	sizerOKCancel->Add( buttonCancel, 0, wxALL, control_border );
    buttonOK->SetDefault();
	return HControl(sizerOKCancel, wxALL | wxALIGN_RIGHT | wxALIGN_BOTTOM);
}

XYZBoxes::XYZBoxes(HDialog *dlg, const wxString& label, const wxString &xstr, const wxString &ystr, const wxString &zstr):wxStaticBoxSizer(wxVERTICAL, dlg, label)
{
	m_sttcX = dlg->AddLabelAndControl(this, xstr, m_lgthX = new CLengthCtrl(dlg));
	m_sttcY = dlg->AddLabelAndControl(this, ystr, m_lgthY = new CLengthCtrl(dlg));
	m_sttcZ = dlg->AddLabelAndControl(this, zstr, m_lgthZ = new CLengthCtrl(dlg));
}
