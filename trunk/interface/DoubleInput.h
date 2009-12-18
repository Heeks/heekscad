// DoubleInput.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/InputMode.h"
#include "../interface/LeftAndRight.h"

class CDoubleInput: public CInputMode, CLeftAndRight
{
public:
	wxString m_title;
	wxString m_value_title;
	double m_value;
	static bool m_success;

	CDoubleInput(const wxChar* prompt, const wxChar* value_name, double initial_value);
	virtual ~CDoubleInput(){}

	// virtual functions for InputMode
	const wxChar* GetTitle();
	void OnMouse( wxMouseEvent& event );
	void GetProperties(std::list<Property *> *list);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
};