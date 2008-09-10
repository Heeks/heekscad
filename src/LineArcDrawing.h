// LineArcDrawing.h

#pragma once

#include "Drawing.h"

enum EnumDrawingMode{
	LineDrawingMode,
	ArcDrawingMode,
	ILineDrawingMode,
	CircleDrawingMode
};

enum EnumCircleDrawingMode{
	CentreAndPointCircleMode,
	ThreePointsCircleMode
};

class LineArcDrawing: public Drawing{
private:
	std::list<HeeksObj*> temp_object_in_list;
	std::list<HeeksObj*> prev_object_in_list;
	HeeksObj *temp_object;
	bool m_previous_direction_set;
	gp_Vec m_previous_direction;
	bool m_A_down; // is key A pressed
	HeeksObj* m_container;

	// Drawing's virtual functions
	void calculate_item(DigitizedPoint &end);
	const std::list<HeeksObj*>& GetObjectsMade()const{return temp_object_in_list;}
	void clear_drawing_objects(bool store_as_previous_objects);
	void set_previous_direction();
	const char* get_drawing_title(){return "Line/Arc Drawing";}
	int number_of_steps();
	int step_to_go_to_after_last_step();
	bool is_an_add_level(int level);
	HeeksObj* GetOwnerForDrawingObjects();
	void AddPoint();

public:
	static wxCursor m_cursor_start;
	static wxCursor m_cursor_end;
	EnumDrawingMode drawing_mode;
	std::list<EnumDrawingMode> m_save_drawing_mode;
	double radius_for_circle;
	EnumCircleDrawingMode circle_mode;

	LineArcDrawing(void);
	virtual ~LineArcDrawing(void);

	// InputMode's virtual functions
	const char* GetTitle(){return "Line and arc drawing";}
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void set_cursor(void);
	void GetTools(std::list<Tool*> *f_list, const wxPoint *p);
	void GetProperties(std::list<Property *> *list);
	void GetOptions(std::list<Property *> *list);
	bool OnModeChange(void);
};

extern LineArcDrawing line_strip;
