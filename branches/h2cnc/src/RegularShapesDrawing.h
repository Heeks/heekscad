// RegularShapesDrawing.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Drawing.h"

enum RegularShapeMode
{
	RectanglesRegularShapeMode,
	PolygonsRegularShapeMode,
	ObroundRegularShapeMode
};

enum PolygonMode
{
	ExcribedMode,
	InscribedMode
};

class RegularShapesDrawing: public Drawing{
private:
	std::list<HeeksObj*> temp_object_in_list;
	std::list<HeeksObj*> prev_object_in_list;
	HeeksObj *temp_object;

	// Drawing's virtual functions
	bool calculate_item(DigitizedPoint &end);
	const std::list<HeeksObj*>& GetObjectsMade()const{return temp_object_in_list;}
	int number_of_steps(){return 2;}
	int step_to_go_to_after_last_step(){return 0;}
	bool is_an_add_level(int level){return level == 1;}

	void CalculateRectangle(double x, double y, const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Dir& xdir, const gp_Dir& ydir, const gp_Dir& zdir);
	void CalculatePolygon(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Dir& zdir);
	void CalculateObround(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Dir& xdir, const gp_Dir& zdir);

public:
	RegularShapeMode m_mode;
	PolygonMode p_mode;
	int m_number_of_side_for_polygon;
	double m_rect_radius;
	double m_obround_radius;

	RegularShapesDrawing(void);
	virtual ~RegularShapesDrawing(void);

	// InputMode's virtual functions
	const wxChar* GetTitle();
	void GetTools(std::list<Tool*> *f_list, const wxPoint *p);
	void GetProperties(std::list<Property *> *list);

	// Drawing's virtual functions
	void clear_drawing_objects(int mode);

	void ClearSketch();
};

extern RegularShapesDrawing regular_shapes_drawing;
