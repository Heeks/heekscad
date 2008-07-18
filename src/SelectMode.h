// SelectMode.h

#pragma once

#include "../interface/InputMode.h"

class CSelectMode: public CInputMode{
public:
	wxPoint CurrentPoint;
	wxPoint button_down_point;
	bool control_key_initially_pressed;
	wxRect window_box;
	bool window_box_exists;
	bool m_doing_a_main_loop;
	std::string m_prompt_when_doing_a_main_loop;
	bool m_right_up_with_left_down_done;
	bool m_left_up_with_right_down_done;

	CSelectMode();
	virtual ~CSelectMode(void){}

	// virtual functions for InputMode
	void OnMouse( wxMouseEvent& event );
	bool OnStart();
	void OnFrontRender();
	void OnRender();
	void GetProperties(std::list<Property *> *list);
	void GetOptions(std::list<Property *> *list);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
};
