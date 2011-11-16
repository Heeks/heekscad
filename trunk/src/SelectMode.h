// SelectMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/InputMode.h"
#include "../interface/LeftAndRight.h"

class CClickPoint
{
	bool m_valid;
	double m_pos[3];
	wxPoint m_point;
	unsigned long m_depth;

public:
	CClickPoint(): m_valid(false){}
	CClickPoint(const wxPoint& point, unsigned long depth);

	bool GetPos(double *pos);
};

class CSelectMode: public CInputMode, CLeftAndRight{
public:
	wxPoint CurrentPoint;
	wxPoint button_down_point;
	bool control_key_initially_pressed;
	bool m_button_down; // don't trust dragging, if button wasn't received first
	bool m_middle_button_down;
	wxRect window_box;
	bool window_box_exists;
	bool m_doing_a_main_loop;
	bool m_just_one;
	wxString m_prompt_when_doing_a_main_loop;
	CClickPoint m_last_click_point;

	CSelectMode();
	virtual ~CSelectMode(void){}

	bool GetLastClickPosition(double *pos);

	// virtual functions for InputMode
	const wxChar* GetTitle();
	bool TitleHighlighted(){return m_doing_a_main_loop;}
	const wxChar* GetHelpText();
	void OnMouse( wxMouseEvent& event );
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	bool OnStart();
	void OnFrontRender();
	void GetProperties(std::list<Property *> *list);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
};
