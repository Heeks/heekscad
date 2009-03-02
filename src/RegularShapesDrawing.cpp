// RegularShapesDrawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "RegularShapesDrawing.h"
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyInt.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"

RegularShapesDrawing regular_shapes_drawing;

RegularShapesDrawing::RegularShapesDrawing(void)
{
	temp_object = NULL;
	m_mode = RectanglesRegularShapeMode;
	m_number_of_side_for_polygon = 6;
	m_rect_radius = 0.0;
	m_obround_radius = 2.0;
}

RegularShapesDrawing::~RegularShapesDrawing(void)
{
}

void RegularShapesDrawing::ClearSketch()
{
	if(temp_object)((CSketch*)temp_object)->Clear();
}

bool RegularShapesDrawing::calculate_item(DigitizedPoint &end)
{
	if(end.m_type == DigitizeNoItemType)return false;

	if(temp_object && temp_object->GetType() != SketchType){
		delete temp_object;
		temp_object = NULL;
		temp_object_in_list.clear();
	}

	// make sure sketch exists
	if(!temp_object){
		temp_object = new CSketch;
		if(temp_object)temp_object_in_list.push_back(temp_object);
	}

	gp_Trsf mat = wxGetApp().GetDrawMatrix(true);
	gp_Dir xdir = gp_Dir(1, 0, 0).Transformed(mat);
	gp_Dir ydir = gp_Dir(0, 1, 0).Transformed(mat);
	gp_Dir zdir = gp_Dir(0, 0, 1).Transformed(mat);

	gp_Pnt p0 = GetStartPos().m_point;
	gp_Pnt p2 = end.m_point;

	double x = gp_Vec(p2.XYZ()) * gp_Vec(xdir.XYZ()) - gp_Vec(p0.XYZ()) * gp_Vec(xdir.XYZ());
	double y = gp_Vec(p2.XYZ()) * gp_Vec(ydir.XYZ()) - gp_Vec(p0.XYZ()) * gp_Vec(ydir.XYZ());

	gp_Pnt p1 = p0.XYZ() + xdir.XYZ() * x;
	gp_Pnt p3 = p0.XYZ() + ydir.XYZ() * y;

	// swap left and right, if user dragged to the left
	if(x < 0){
		gp_Pnt t = p0;
		p0 = p1;
		p1 = t;
		t = p3;
		p3 = p2;
		p2 = t;
	}

	// swap top and bottom, if user dragged upward
	if(y < 0){
		gp_Pnt t = p0;
		p0 = p3;
		p3 = t;
		t = p1;
		p1 = p2;
		p2 = t;
	}

	// add ( or modify ) lines and arcs
	switch(m_mode)
	{
	case RectanglesRegularShapeMode:
		CalculateRectangle(x, y, p0, p1, p2, p3, xdir, ydir, zdir);
		break;
	case PolygonsRegularShapeMode:
		CalculatePolygon(GetStartPos().m_point, end.m_point, zdir);
		break;
	case ObroundRegularShapeMode:
		CalculateObround(GetStartPos().m_point, end.m_point, xdir, zdir);
		break;
	}

	return true;
}

void RegularShapesDrawing::CalculateRectangle(double x, double y, const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Dir& xdir, const gp_Dir& ydir, const gp_Dir& zdir)
{
	bool radii_wanted = false;
	bool x_lines_disappear = false;
	bool y_lines_disappear = false;

	if(m_rect_radius > 0.0000000001)
	{
		if(fabs(x) - m_rect_radius*2 > -0.0000000001 && fabs(y) - m_rect_radius*2 > -0.0000000001)
			radii_wanted = true;
		if(fabs(fabs(x) - m_rect_radius*2) < 0.0000000001)
			x_lines_disappear = true;
		if(fabs(fabs(y) - m_rect_radius*2) < 0.0000000001)
			y_lines_disappear = true;
	}

	int good_num = 4;

	if(radii_wanted)
	{
		if(x_lines_disappear && y_lines_disappear)good_num = 2;
		else if(x_lines_disappear || y_lines_disappear)good_num = 4;
		else good_num = 8;
	}

	if(temp_object->GetNumChildren() != good_num)ClearSketch();
	// check first item
	else if(temp_object->GetFirstChild()->GetType() != (radii_wanted ? ArcType:LineType))
		ClearSketch();

	if(radii_wanted)
	{
		if(x_lines_disappear && y_lines_disappear)
		{
			// make two arcs, for a circle
			HArc* arcs[2];
			if(temp_object->GetNumChildren() > 0)
			{
				HeeksObj* object = temp_object->GetFirstChild();
				for(int i = 0; i<2; i++)
				{
					arcs[i] = (HArc*)object;
					object = temp_object->GetNextChild();
				}
			}
			else
			{
				for(int i = 0; i<2; i++)
				{
					arcs[i] = new HArc(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), gp_Circ(), &(wxGetApp().current_color));
					temp_object->Add(arcs[i], NULL);
				}
			}
			arcs[0]->A = p0.XYZ() + xdir.XYZ() * m_rect_radius;
			arcs[0]->B = p3.XYZ() + xdir.XYZ() * m_rect_radius;
			arcs[0]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p0.XYZ() + xdir.XYZ() * m_rect_radius + ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
			arcs[1]->A = arcs[0]->B;
			arcs[1]->B = arcs[0]->A;
			arcs[1]->m_circle = arcs[0]->m_circle;
		}
		else if(x_lines_disappear || y_lines_disappear)
		{
			// arc-line-arc-line
			HArc* arcs[2];
			HLine* lines[2];
			if(temp_object->GetNumChildren() > 0)
			{
				HeeksObj* object = temp_object->GetFirstChild();
				for(int i = 0; i<2; i++)
				{
					arcs[i] = (HArc*)object;
					object = temp_object->GetNextChild();
					lines[i] = (HLine*)object;
					object = temp_object->GetNextChild();
				}
			}
			else
			{
				for(int i = 0; i<2; i++)
				{
					arcs[i] = new HArc(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), gp_Circ(), &(wxGetApp().current_color));
					temp_object->Add(arcs[i], NULL);
					lines[i] = new HLine(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), &(wxGetApp().current_color));
					temp_object->Add(lines[i], NULL);
				}
			}

			if(x_lines_disappear){
				arcs[0]->A = p2.XYZ() - ydir.XYZ() * m_rect_radius;
				arcs[0]->B = p3.XYZ() - ydir.XYZ() * m_rect_radius;
				arcs[0]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p3.XYZ() + xdir.XYZ() * m_rect_radius - ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
				lines[0]->A = arcs[0]->B;
				lines[0]->B = p0.XYZ() + ydir.XYZ() * m_rect_radius;
				arcs[1]->A = lines[0]->B;
				arcs[1]->B = p1.XYZ() + ydir.XYZ() * m_rect_radius;
				arcs[1]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p0.XYZ() + xdir.XYZ() * m_rect_radius + ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
				lines[1]->A = arcs[1]->B;
				lines[1]->B = arcs[0]->A;
			}
			else{
				arcs[0]->A = p1.XYZ() - xdir.XYZ() * m_rect_radius;
				arcs[0]->B = p2.XYZ() - xdir.XYZ() * m_rect_radius;
				arcs[0]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p1.XYZ() - xdir.XYZ() * m_rect_radius + ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
				lines[0]->A = arcs[0]->B;
				lines[0]->B = p3.XYZ() + xdir.XYZ() * m_rect_radius;
				arcs[1]->A = lines[0]->B;
				arcs[1]->B = p0.XYZ() + xdir.XYZ() * m_rect_radius;
				arcs[1]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p0.XYZ() + xdir.XYZ() * m_rect_radius + ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
				lines[1]->A = arcs[1]->B;
				lines[1]->B = arcs[0]->A;
			}
		}
		else{
			// arc-line-arc-line-arc-line-arc-line
			HLine* lines[4];
			HArc* arcs[4];
			if(temp_object->GetNumChildren() > 0)
			{
				HeeksObj* object = temp_object->GetFirstChild();
				for(int i = 0; i<4; i++)
				{
					arcs[i] = (HArc*)object;
					object = temp_object->GetNextChild();
					lines[i] = (HLine*)object;
					object = temp_object->GetNextChild();
				}
			}
			else
			{
				for(int i = 0; i<4; i++)
				{
					arcs[i] = new HArc(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), gp_Circ(), &(wxGetApp().current_color));
					temp_object->Add(arcs[i], NULL);
					lines[i] = new HLine(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), &(wxGetApp().current_color));
					temp_object->Add(lines[i], NULL);
				}
			}

			arcs[0]->A = p1.XYZ() - xdir.XYZ() * m_rect_radius;
			arcs[0]->B = p1.XYZ() + ydir.XYZ() * m_rect_radius;
			arcs[0]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p1.XYZ() - xdir.XYZ() * m_rect_radius + ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
			lines[0]->A = arcs[0]->B;
			lines[0]->B = p2.XYZ() - ydir.XYZ() * m_rect_radius;
			arcs[1]->A = lines[0]->B;
			arcs[1]->B = p2.XYZ() - xdir.XYZ() * m_rect_radius;
			arcs[1]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p2.XYZ() - xdir.XYZ() * m_rect_radius - ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
			lines[1]->A = arcs[1]->B;
			lines[1]->B = p3.XYZ() + xdir.XYZ() * m_rect_radius;
			arcs[2]->A = lines[1]->B;
			arcs[2]->B = p3.XYZ() - ydir.XYZ() * m_rect_radius;
			arcs[2]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p3.XYZ() + xdir.XYZ() * m_rect_radius - ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
			lines[2]->A = arcs[2]->B;
			lines[2]->B = p0.XYZ() + ydir.XYZ() * m_rect_radius;
			arcs[3]->A = lines[2]->B;
			arcs[3]->B = p0.XYZ() + xdir.XYZ() * m_rect_radius;
			arcs[3]->m_circle = gp_Circ(gp_Ax2(gp_Pnt(p0.XYZ() + xdir.XYZ() * m_rect_radius + ydir.XYZ() * m_rect_radius), zdir), m_rect_radius);
			lines[3]->A = arcs[3]->B;
			lines[3]->B = arcs[0]->A;
		}
	}
	else
	{
		// line-line-line-line
		HLine* lines[4];
		if(temp_object->GetNumChildren() > 0)
		{
			HeeksObj* object = temp_object->GetFirstChild();
			for(int i = 0; i<4; i++)
			{
				lines[i] = (HLine*)object;
				object = temp_object->GetNextChild();
			}
		}
		else
		{
			for(int i = 0; i<4; i++)
			{
				lines[i] = new HLine(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), &(wxGetApp().current_color));
				temp_object->Add(lines[i], NULL);
			}
		}

		lines[0]->A = p0;
		lines[0]->B = p1;
		lines[1]->A = p1;
		lines[1]->B = p2;
		lines[2]->A = p2;
		lines[2]->B = p3;
		lines[3]->A = p3;
		lines[3]->B = p0;
	}
}

void RegularShapesDrawing::CalculatePolygon(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Dir& zdir)
{
	if(p0.IsEqual(p1, wxGetApp().m_geom_tol))return;

	if(temp_object->GetNumChildren() != m_number_of_side_for_polygon)ClearSketch();
	{
		HLine** lines = (HLine**)malloc(m_number_of_side_for_polygon * sizeof(HLine*));

		if(temp_object->GetNumChildren() > 0)
		{
			HeeksObj* object = temp_object->GetFirstChild();
			for(int i = 0; i<m_number_of_side_for_polygon; i++)
			{
				lines[i] = (HLine*)object;
				object = temp_object->GetNextChild();
			}
		}
		else
		{
			for(int i = 0; i<m_number_of_side_for_polygon; i++)
			{
				lines[i] = new HLine(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), &(wxGetApp().current_color));
				temp_object->Add(lines[i], NULL);
			}
		}

		double radius = p0.Distance(p1);

		for(int i = 0; i<m_number_of_side_for_polygon; i++)
		{
			gp_Dir xdir(make_vector(p0, p1));
			gp_Dir ydir = zdir ^ xdir;
			double angle0 = 2.0 * Pi / m_number_of_side_for_polygon * i;
			double angle1 = 2.0 * Pi / m_number_of_side_for_polygon * (i+1);
			lines[i]->A = p0.XYZ() + xdir.XYZ() * ( cos(angle0) * radius ) + ydir.XYZ() * ( sin(angle0) * radius );
			if(i == m_number_of_side_for_polygon - 1)lines[i]->B = lines[0]->A;
			lines[i]->B = p0.XYZ() + xdir.XYZ() * ( cos(angle1) * radius ) + ydir .XYZ()* ( sin(angle1) * radius );
		}
	}
}

void RegularShapesDrawing::CalculateObround(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Dir& xdir, const gp_Dir& zdir)
{
	bool lines_disappear = false;

	if(m_obround_radius > 0.0000000001)
	{
		if(p0.IsEqual(p1, wxGetApp().m_geom_tol))lines_disappear = true;
	}
	else return;

	int good_num = 4;
	if(lines_disappear)good_num = 2;

	if(temp_object->GetNumChildren() != good_num)ClearSketch();

	if(lines_disappear)
	{
		// make two arcs, for a circle
		HArc* arcs[2];
		if(temp_object->GetNumChildren() > 0)
		{
			HeeksObj* object = temp_object->GetFirstChild();
			for(int i = 0; i<2; i++)
			{
				arcs[i] = (HArc*)object;
				object = temp_object->GetNextChild();
			}
		}
		else
		{
			for(int i = 0; i<2; i++)
			{
				arcs[i] = new HArc(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), gp_Circ(), &(wxGetApp().current_color));
				temp_object->Add(arcs[i], NULL);
			}
		}
		arcs[0]->A = p0.XYZ() + xdir.XYZ() * m_obround_radius;
		arcs[0]->B = p0.XYZ() - xdir.XYZ() * m_obround_radius;
		arcs[0]->m_circle = gp_Circ(gp_Ax2(p0, zdir), m_obround_radius);
		arcs[1]->A = arcs[0]->B;
		arcs[1]->B = arcs[0]->A;
		arcs[1]->m_circle = arcs[0]->m_circle;
	}
	else
	{
		// arc-line-arc-line
		HArc* arcs[2];
		HLine* lines[2];
		if(temp_object->GetNumChildren() > 0)
		{
			HeeksObj* object = temp_object->GetFirstChild();
			for(int i = 0; i<2; i++)
			{
				arcs[i] = (HArc*)object;
				object = temp_object->GetNextChild();
				lines[i] = (HLine*)object;
				object = temp_object->GetNextChild();
			}
		}
		else
		{
			for(int i = 0; i<2; i++)
			{
				arcs[i] = new HArc(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), gp_Circ(), &(wxGetApp().current_color));
				temp_object->Add(arcs[i], NULL);
				lines[i] = new HLine(gp_Pnt(0, 0, 0), gp_Pnt(0, 0, 0), &(wxGetApp().current_color));
				temp_object->Add(lines[i], NULL);
			}
		}

		gp_Dir along_dir(make_vector(p0, p1));
		gp_Dir right_dir = along_dir ^ zdir;

		arcs[0]->A = p1.XYZ() + right_dir.XYZ() * m_obround_radius;
		arcs[0]->B = p1.XYZ() - right_dir.XYZ() * m_obround_radius;
		arcs[0]->m_circle = gp_Circ(gp_Ax2(p1, zdir), m_obround_radius);
		lines[0]->A = arcs[0]->B;
		lines[0]->B = p0.XYZ() - right_dir.XYZ() * m_obround_radius;
		arcs[1]->A = lines[0]->B;
		arcs[1]->B = p0.XYZ() + right_dir.XYZ() * m_obround_radius;
		arcs[1]->m_circle = gp_Circ(gp_Ax2(p0, zdir), m_obround_radius);
		lines[1]->A = arcs[1]->B;
		lines[1]->B = arcs[0]->A;
	}
}

void RegularShapesDrawing::clear_drawing_objects(int mode)
{
	if(mode == 2 && temp_object)delete temp_object;
	temp_object = NULL;
	temp_object_in_list.clear();
}

static RegularShapesDrawing* RegularShapesDrawing_for_GetProperties = NULL;

static void on_set_drawing_mode(int value, HeeksObj* object)
{
	RegularShapesDrawing_for_GetProperties->m_mode = (RegularShapeMode)value;
	RegularShapesDrawing_for_GetProperties->ClearSketch();
	wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
}

static void on_set_rect_radius(double value, HeeksObj* object)
{
	RegularShapesDrawing_for_GetProperties->m_rect_radius = value;
}

static void on_set_obround_radius(double value, HeeksObj* object)
{
	RegularShapesDrawing_for_GetProperties->m_obround_radius = value;
}

static void on_set_num_sides(int value, HeeksObj* object)
{
	RegularShapesDrawing_for_GetProperties->m_number_of_side_for_polygon = value;
}

void RegularShapesDrawing::GetProperties(std::list<Property *> *list){
	// add drawing mode
	std::list< wxString > choices;
	choices.push_back ( wxString ( _("draw rectangles") ) );
	choices.push_back ( wxString ( _("draw polygons") ) );
	choices.push_back ( wxString ( _("draw slots") ) );
	RegularShapesDrawing_for_GetProperties = this;
	list->push_back ( new PropertyChoice ( _("drawing mode"),  choices, m_mode, NULL, on_set_drawing_mode ) );


	if(m_mode == RectanglesRegularShapeMode)list->push_back( new PropertyDouble( _("radius"), m_rect_radius, NULL, on_set_rect_radius));
	if(m_mode == ObroundRegularShapeMode)list->push_back( new PropertyDouble( _("radius"), m_obround_radius, NULL, on_set_obround_radius));
	if(m_mode == PolygonsRegularShapeMode)list->push_back( new PropertyInt(_("number of sides for polygon"), m_number_of_side_for_polygon, NULL, on_set_num_sides));

	Drawing::GetProperties(list);
}

void RegularShapesDrawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	Drawing::GetTools(f_list, p);
}
