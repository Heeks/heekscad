// DimensionDrawing.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Drawing.h"
#include "HDimension.h"

class DimensionDrawing: public Drawing{
private:
	std::list<HeeksObj*> temp_object_in_list;
	std::list<HeeksObj*> prev_object_in_list;
	HeeksObj *temp_object;

	// Drawing's virtual functions
	bool calculate_item(DigitizedPoint &end);
	const std::list<HeeksObj*>& GetObjectsMade()const{return temp_object_in_list;}
	int number_of_steps(){return 3;}
	int step_to_go_to_after_last_step(){return 0;}
	bool is_an_add_level(int level){return level == 2;}
	bool is_a_draw_level(int level){return level >= 1;}

public:
	DimensionMode m_mode;

	DimensionDrawing(void);
	virtual ~DimensionDrawing(void);

	// InputMode's virtual functions
	const wxChar* GetTitle(){return _("Dimension drawing");}
	void GetTools(std::list<Tool*> *f_list, const wxPoint *p);
	void GetProperties(std::list<Property *> *list);

	// Drawing's virtual functions
	void clear_drawing_objects(int mode);
	bool DragDoneWithXOR(){return false;}

	void StartOnStep3(HDimension* object);
};

extern DimensionDrawing dimension_drawing;
