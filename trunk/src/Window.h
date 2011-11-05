// Window.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/InputMode.h"

class WindowDragging: public CInputMode{
public:
	wxRect window_box;
	bool finish_dragging;
	bool box_found;
	bool box_drawn_with_cross;

	WindowDragging();

	// virtual functions from InputMode
	const wxChar* GetTitle(){return _("Dragging a window");}
	void OnMouse( wxMouseEvent& event );

	void reset(void);
};
