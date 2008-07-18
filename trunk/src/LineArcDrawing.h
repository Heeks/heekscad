// LineArcDrawing.h

#pragma once

#include "Drawing.h"

class LineArcDrawing: public Drawing{
private:
	std::list<HeeksObj*> temp_object_in_list;
	HeeksObj *temp_object;
	gp_Vec m_previous_direction;
	bool m_A_down; // is key A pressed
	HeeksObj* m_container;

	// Drawing's virtual functions
	void calculate_item(const gp_Pnt &end);
	const std::list<HeeksObj*>& GetObjectsMade()const{return temp_object_in_list;}
	void clear_drawing_objects();
	void set_previous_direction();
	const char* get_drawing_title(){return "Line/Arc Drawing";}
	int number_of_steps(){return 2;}
	int step_to_go_to_after_last_step(){return 1;}
	bool is_an_add_level(int level){if(level == 1)return true; return false;}
	HeeksObj* GetOwnerForDrawingObjects();

public:
	static wxCursor m_cursor_start;
	static wxCursor m_cursor_end;
	int drawing_mode; // 0 - lines, 1 - arcs
	std::list<int> m_save_drawing_mode;

	LineArcDrawing(void);
	virtual ~LineArcDrawing(void);

	// InputMode's virtual functions
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void set_cursor(void);
	void GetTools(std::list<Tool*> *f_list, const wxPoint *p);
	void GetProperties(std::list<Property *> *list);
	void GetOptions(std::list<Property *> *list);
	bool OnModeChange(void);
};

extern LineArcDrawing line_strip;
