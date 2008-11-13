// Window.h

#pragma once

#include "../interface/InputMode.h"

class Window: public CInputMode{
public:
	wxRect window_box;
	bool finish_dragging;
	bool box_found;
	bool box_drawn_with_cross;

	Window();

	// virtual functions from InputMode
	const wxChar* GetTitle(){return _("Dragging a window");}
	void OnMouse( wxMouseEvent& event );

	void reset(void);
};
