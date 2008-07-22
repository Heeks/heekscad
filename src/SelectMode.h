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
	std::string m_prompt_when_doing_a_main_loop;

	CSelectMode();
	virtual ~CSelectMode(void){}

	// virtual functions for InputMode
	const char* GetTitle(){return "Picking objects";}
	bool TitleHighlighted(){return m_doing_a_main_loop;}
	void OnMouse( wxMouseEvent& event );
	bool OnStart();
	void OnFrontRender();
	void OnRender();
	void GetProperties(std::list<Property *> *list);
	void GetOptions(std::list<Property *> *list);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
};
