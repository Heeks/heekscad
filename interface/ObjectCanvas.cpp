// ObjectCanvas.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "ObjectCanvas.h"
#include "Tool.h"

BEGIN_EVENT_TABLE(TextCanvas, ObjectCanvas)
    EVT_TEXT(wxID_ANY,TextCanvas::OnTextCtrlEvent)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MyTextCtrl, wxTextCtrl)
    EVT_KILL_FOCUS(MyTextCtrl::OnKillFocusEvent)
END_EVENT_TABLE()

TextCanvas::TextCanvas(wxWindow* parent, wxString *str) : ObjectCanvas(parent)
{
	m_text = new MyTextCtrl(this, str);
	m_sizer = new wxBoxSizer(wxVERTICAL);
	m_sizer->Add(m_text, 1, wxGROW);
	SetSizer(m_sizer);
}

void TextCanvas::SetWithObject(HeeksObj* object)
{
	m_text->ChangeValue(*m_text->m_str);
}

void TextCanvas::OnTextCtrlEvent(wxCommandEvent& event)
{
}

TextCanvas* TextCanvas::global_text_canvas = NULL;

class TextSetUndoable: public Undoable{
public:
	wxString m_old_str;
	wxString m_new_str;
	wxString *m_pstr;

	TextSetUndoable(const wxString &old_str, const wxString &new_str, wxString *str)
	{
		m_old_str = old_str;
		m_new_str = new_str;
		m_pstr = str;
	}

	void Run(bool redo)
	{
		*m_pstr = m_new_str;
		if(TextCanvas::global_text_canvas)TextCanvas::global_text_canvas->m_text->ChangeValue(*m_pstr);
	}

	const wxChar* GetTitle()
	{
		return _T("TextSetUndoable");
	}

	void RollBack()
	{
		*m_pstr = m_old_str;
		if(TextCanvas::global_text_canvas)TextCanvas::global_text_canvas->m_text->ChangeValue(*m_pstr);
	}

};

void MyTextCtrl::OnKillFocusEvent( wxFocusEvent& event )
{
	heeksCAD->DoUndoable( new TextSetUndoable(m_original_text, GetValue(), m_str));
}
