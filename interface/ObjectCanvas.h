// ObjectCanvas.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "PictureFrame.h"

// base class for windows to appear in the "Properties" window
class ObjectCanvas: public wxScrolledWindow
{
public:
	ObjectCanvas(wxWindow* parent): wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE){}
	virtual ~ObjectCanvas(){}

public:
	virtual void SetWithObject(HeeksObj* object){}
	virtual void OnApply(){}
	virtual void OnCancel(){};
};

// a class which simply has a picture
class PictureCanvas: public ObjectCanvas
{
public:
	PictureFrame* m_picture;
	wxBoxSizer* m_sizer;
	PictureFrame* current_picture;

	PictureCanvas(wxWindow* parent, const wxBitmap& bitmap) : ObjectCanvas(parent)
	{
		// just draw a picture
		m_picture = new PictureFrame(this, bitmap);
		m_sizer = new wxBoxSizer(wxVERTICAL);
		m_sizer->Add(m_picture, 1, wxGROW);
		current_picture = m_picture;
		SetSizer(m_sizer);
	}

	void SetPicture(PictureFrame* picture)
	{
		if(picture != current_picture)
		{
			m_sizer->Detach(current_picture);
			current_picture->Show(false);
			m_sizer->Add(picture, 1, wxGROW);
			picture->Show();
			current_picture = picture;
			m_sizer->RecalcSizes();
			Refresh();
		}
	}

	void SetWithObject(HeeksObj* object)
	{
		// use the main picture
		SetPicture(m_picture);
	}
};

// a class which simply has text
class TextCanvas: public ObjectCanvas
{
public:
	wxString* m_str;
	wxTextCtrl* m_text;
	wxBoxSizer* m_sizer;

	TextCanvas(wxWindow* parent, wxString *str);
	void SetWithObject(HeeksObj* object);
	void OnTextCtrlEvent(wxCommandEvent& event);
	void OnKillFocusEvent( wxFocusEvent& event );

    DECLARE_NO_COPY_CLASS(TextCanvas)
	DECLARE_EVENT_TABLE()
};
