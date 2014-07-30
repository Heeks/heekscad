// ObjectCanvas.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "ObjectCanvas.h"

BEGIN_EVENT_TABLE(TextCanvas, ObjectCanvas)
    EVT_TEXT(wxID_ANY,TextCanvas::OnTextCtrlEvent)
    EVT_KILL_FOCUS(TextCanvas::OnKillFocusEvent)
END_EVENT_TABLE()

TextCanvas::TextCanvas(wxWindow* parent, wxString *str) : ObjectCanvas(parent), m_str(str)
{
	m_text = new wxTextCtrl(this, wxID_ANY, *str, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	m_sizer = new wxBoxSizer(wxVERTICAL);
	m_sizer->Add(m_text, 1, wxGROW);
	SetSizer(m_sizer);
}

void TextCanvas::SetWithObject(HeeksObj* object)
{
	m_text->ChangeValue(*m_str);
}

void TextCanvas::OnTextCtrlEvent(wxCommandEvent& event)
{
	*m_str = m_text->GetValue();
}

void TextCanvas::OnKillFocusEvent( wxFocusEvent& event )
{
	*m_str = m_text->GetValue();
}
