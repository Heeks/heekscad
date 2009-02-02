// SelectMode.h

#pragma once

#include "../interface/InputMode.h"
#include "../interface/LeftAndRight.h"

class CSelectMode: public CInputMode, CLeftAndRight{
public:
	wxPoint CurrentPoint;
	wxPoint button_down_point;
	bool control_key_initially_pressed;
	wxRect window_box;
	bool window_box_exists;
	bool m_doing_a_main_loop;
	bool m_just_one;
	wxString m_prompt_when_doing_a_main_loop;

	static bool m_can_grip_objects;

	CSelectMode();
	virtual ~CSelectMode(void){}

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
