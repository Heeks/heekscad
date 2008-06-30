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

	CSelectMode();
	virtual ~CSelectMode(void){}

	// virtual functions for InputMode
	void OnMouse( wxMouseEvent& event );
	bool start();
	void OnFrontRender();
	void OnRender();
	void GetProperties(std::list<Property *> *list);
	void GetSharedProperties(std::list<Property *> *list);

	virtual void mouse_move2(const wxPoint &point){};
	virtual void left_button_down2(const wxPoint &point){};
	virtual void set_cursor2(void){}
};
