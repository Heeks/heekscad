// LineArcDrawing.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Drawing.h"

enum EnumDrawingMode{
	LineDrawingMode,
	ArcDrawingMode,
	ILineDrawingMode,
	CircleDrawingMode,
	EllipseDrawingMode,
	SplineDrawingMode
};

enum EnumCircleDrawingMode{
	CentreAndPointCircleMode,
	ThreePointsCircleMode,
	TwoPointsCircleMode,
	CentreAndRadiusCircleMode // only one click needed ( edit radius in the properties before clicking)
};

enum EnumSplineDrawingMode{
	CubicSplineMode,
	QuarticSplineMode,
	RationalSplineMode
};


class LineArcDrawing: public Drawing{
private:
	std::list<HeeksObj*> temp_object_in_list;
	std::list<HeeksObj*> prev_object_in_list;
	std::list<DigitizedPoint> spline_points;
	HeeksObj *temp_object;
	bool m_A_down; // is key A pressed
	HeeksObj* m_container;
	bool m_add_to_sketch;

	// Drawing's virtual functions
	bool calculate_item(DigitizedPoint &end);
	const std::list<HeeksObj*>& GetObjectsMade()const{return temp_object_in_list;}
	void set_previous_direction();
	int number_of_steps();
	int step_to_go_to_after_last_step();
	bool is_an_add_level(int level);
	bool is_a_draw_level(int level);
	HeeksObj* GetOwnerForDrawingObjects();
	void AddPoint();

public:
	static wxCursor m_cursor_start;
	static wxCursor m_cursor_end;
	EnumDrawingMode drawing_mode;
	std::list<EnumDrawingMode> m_save_drawing_mode;
	double radius_for_circle;
	EnumCircleDrawingMode circle_mode;
	EnumSplineDrawingMode spline_mode;
	bool m_previous_direction_set;
	gp_Vec m_previous_direction;

	LineArcDrawing(void);
	virtual ~LineArcDrawing(void);

	// InputMode's virtual functions
	const wxChar* GetTitle();
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void set_cursor(void);
	void GetTools(std::list<Tool*> *f_list, const wxPoint *p);
	void GetProperties(std::list<Property *> *list);
	bool OnModeChange(void);

	// Drawing's virtual functions
	void clear_drawing_objects(int mode);
	void set_draw_step_not_undoable(int s);
};

extern LineArcDrawing line_strip;
