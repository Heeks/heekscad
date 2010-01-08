// LeftAndRight.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class CLeftAndRight{
	// derive your input mode derived class from this and call LeftAndRightPressed from OnMouse

protected:
	bool m_right_up_with_left_down_done;
	bool m_left_up_with_right_down_done;

public:
	CLeftAndRight();

	bool LeftAndRightPressed(wxMouseEvent& event, bool &event_used);
};
