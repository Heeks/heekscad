// HeeksObjDlg.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HeeksObjDlg.h"
#include "PictureFrame.h"
#include "NiceTextCtrl.h"

BEGIN_EVENT_TABLE(HeeksObjDlg, HDialog)
    EVT_CHILD_FOCUS(HeeksObjDlg::OnChildFocus)
END_EVENT_TABLE()

HeeksObjDlg::HeeksObjDlg(wxWindow *parent, HeeksObj* object, const wxString& title, bool top_level, bool picture)
             : HDialog(parent, wxID_ANY, title)
{
	m_object = object;

	// add picture to right side
	if(picture)
	{
		m_picture = new PictureWindow(this, wxSize(300, 200));
		wxBoxSizer *pictureSizer = new wxBoxSizer(wxVERTICAL);
		pictureSizer->Add(m_picture, 1, wxGROW);
		rightControls.push_back(HControl(pictureSizer, wxALL));
	}
	else
	{
		m_picture = NULL;
	}

	if(top_level)
	{
		HeeksObjDlg::AddControlsAndCreate();
		m_picture->SetFocus();
	}
}

void HeeksObjDlg::GetData(HeeksObj* object)
{
	if(m_ignore_event_functions)return;
	m_ignore_event_functions = true;
	GetDataRaw(object);
	m_ignore_event_functions = false;
}

void HeeksObjDlg::SetFromData(HeeksObj* object)
{
	bool save_ignore_event_functions = m_ignore_event_functions;
	m_ignore_event_functions = true;
	SetFromDataRaw(object);
	m_ignore_event_functions = save_ignore_event_functions;
}

void HeeksObjDlg::GetDataRaw(HeeksObj* object)
{
}

void HeeksObjDlg::SetFromDataRaw(HeeksObj* object)
{
}

void HeeksObjDlg::SetPicture(const wxString& name)
{
	SetPicture(name, _T("heeksobj"));
}

void HeeksObjDlg::SetPicture(const wxString& name, const wxString& folder)
{
	if(m_picture)m_picture->SetPicture(
#ifdef HEEKSCAD
		wxGetApp().GetResFolder()
#else
		theApp.GetResFolder()
#endif
		+ _T("/bitmaps/") + folder + _T("/") + name + _T(".png"), wxBITMAP_TYPE_PNG);
}

void HeeksObjDlg::SetPictureByWindow(wxWindow* w)
{
	SetPicture(_T("general"));
}

void HeeksObjDlg::SetPicture()
{
	wxWindow* w = FindFocus();

	SetPictureByWindow(w);

}

void HeeksObjDlg::OnChildFocus(wxChildFocusEvent& event)
{
	if(m_ignore_event_functions)return;
	if(event.GetWindow())
	{
		SetPicture();
	}
}

void HeeksObjDlg::OnComboOrCheck( wxCommandEvent& event )
{
	if(m_ignore_event_functions)return;
	SetPicture();
}

void HeeksObjDlg::AddControlsAndCreate()
{
	m_ignore_event_functions = true;
    wxBoxSizer *sizerMain = new wxBoxSizer(wxHORIZONTAL);

	// add left sizer
    wxBoxSizer *sizerLeft = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add( sizerLeft, 0, wxALL, control_border );

	// add left controls
	for(std::list<HControl>::iterator It = leftControls.begin(); It != leftControls.end(); It++)
	{
		HControl c = *It;
		c.AddToSizer(sizerLeft);
	}

	// add right sizer
    wxBoxSizer *sizerRight = new wxBoxSizer(wxVERTICAL);
    sizerMain->Add( sizerRight, 0, wxALL, control_border );

	// add OK and Cancel to right side
	rightControls.push_back(MakeOkAndCancel(wxHORIZONTAL));

	// add right controls
	for(std::list<HControl>::iterator It = rightControls.begin(); It != rightControls.end(); It++)
	{
		HControl c = *It;
		c.AddToSizer(sizerRight);
	}

	SetFromData(m_object);

    SetSizer( sizerMain );
    sizerMain->SetSizeHints(this);
	sizerMain->Fit(this);

	m_ignore_event_functions = false;

	HeeksObjDlg::SetPicture();
}