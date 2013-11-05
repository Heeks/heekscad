// PointDrawing.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Drawing.h"

class PointDrawing: public Drawing{
private:
	// Drawing's virtual functions
	bool calculate_item(DigitizedPoint &end);
	int number_of_steps(){return 1;}
	int step_to_go_to_after_last_step(){return 0;}
	bool is_an_add_level(int level){return true;}

public:
	PointDrawing(void);
	virtual ~PointDrawing(void);

	// InputMode's virtual functions
	const wxChar* GetTitle(){return _("Point drawing");}
};

extern PointDrawing point_drawing;
