// DoubleInput.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "DoubleInput.h"
#include "PropertyDouble.h"
#include "PropertyLength.h"

static double* value_for_set_value = NULL;
static void set_value(double value, HeeksObj* object){*value_for_set_value = value;}

class CInputApply:public Tool{
public:
	void Run(){
		CDoubleInput::m_success = true;
		wxGetApp().ExitMainLoop();
	}
	const wxChar* GetTitle(){return _("Apply");}
	wxString BitmapPath(){return _T("apply");}
	const wxChar* GetToolTip(){return _("Accept value and continue");}
};

CInputApply input_apply;

class CInputCancel:public Tool{
public:
	void Run(){wxGetApp().ExitMainLoop();}
	const wxChar* GetTitle(){return _("Cancel");}
	wxString BitmapPath(){return _T("cancel");}
	const wxChar* GetToolTip(){return _("Cancel operation");}
};

CInputCancel input_cancel;

// static
bool CDoubleInput::m_success = false;

CDoubleInput::CDoubleInput(const wxChar* prompt, const wxChar* value_name, double initial_value)
{
	m_title.assign(prompt);
	m_value_title.assign(value_name);
	m_value = initial_value;
	m_success = false;
}

const wxChar* CDoubleInput::GetTitle()
{
	return m_title.c_str();
}

void CDoubleInput::OnMouse( wxMouseEvent& event )
{
	bool event_used = false;
	if(LeftAndRightPressed(event, event_used))
		wxGetApp().ExitMainLoop();
}

void CDoubleInput::GetProperties(std::list<Property *> *list)
{
	value_for_set_value = &m_value;
	list->push_back(new PropertyDouble(m_value_title.c_str(), m_value, NULL, set_value));
}


void CDoubleInput::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	// add a do it now button
	t_list->push_back(&input_apply);
	t_list->push_back(&input_cancel);
}


CLengthInput::CLengthInput(const wxChar* prompt, const wxChar* value_name, double initial_value)
    : CDoubleInput(prompt, value_name, initial_value) { }

void CLengthInput::GetProperties(std::list<Property *> *list)
{
	value_for_set_value = &m_value;
	list->push_back(new PropertyLength(m_value_title.c_str(), m_value, NULL, set_value));
}

