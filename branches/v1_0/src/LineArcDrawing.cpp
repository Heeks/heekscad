// LineArcDrawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "LineArcDrawing.h"
#include "../interface/HeeksObj.h"
#include "../interface/Tool.h"
#include "HLine.h"
#include "HArc.h"
#include "HILine.h"
#include "HCircle.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyString.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"
#include "ObjPropsCanvas.h"
#include "Sketch.h"
#include "GraphicsCanvas.h"
#include "HeeksConfig.h"

wxCursor LineArcDrawing::m_cursor_start;
wxCursor LineArcDrawing::m_cursor_end;

LineArcDrawing line_strip;

LineArcDrawing::LineArcDrawing(void){
	m_previous_direction_set = false;
	m_previous_direction = gp_Vec(1, 0, 0);
	drawing_mode = LineDrawingMode;
	m_A_down = false;
	m_container = NULL;
	radius_for_circle = 5.0;
	circle_mode = ThreePointsCircleMode;
	m_add_to_sketch = true;
}

LineArcDrawing::~LineArcDrawing(void){
}

class SetPreviousDirection:public Undoable{
private:
	LineArcDrawing *drawing;
	gp_Vec old_direction;
	gp_Vec new_direction;
	bool old_previous_direction_set;

public:
	SetPreviousDirection(LineArcDrawing *d, const gp_Vec& n)
	{
		drawing = d;
		old_direction = drawing->m_previous_direction;
		new_direction = n;
		old_previous_direction_set = drawing->m_previous_direction_set;
	}

	// Tool's virtual functions
	const wxChar* GetTitle(){return _("set previous direction");}
	void Run(bool redo)
	{
		drawing->m_previous_direction = new_direction;
		drawing->m_previous_direction_set = true;
	}
	void RollBack()
	{
		if(old_previous_direction_set)drawing->m_previous_direction = old_direction;
		drawing->m_previous_direction_set = old_previous_direction_set;
	}
};

void LineArcDrawing::set_previous_direction(){
	if(PrevObject() == NULL)return;

	if(PrevObject()->GetType() == LineType){
		double s[3], e[3];
		if(PrevObject()->GetStartPoint(s) && PrevObject()->GetEndPoint(e))
		{
			wxGetApp().DoUndoable(new SetPreviousDirection(this, make_vector(make_point(s), make_point(e))));
		}
	}
	else if(PrevObject()->GetType() == ArcType){
		gp_Vec circlev(((HArc*)PrevObject())->m_axis.Direction());
		gp_Vec endv(((HArc*)PrevObject())->C, ((HArc*)PrevObject())->B);
		wxGetApp().DoUndoable(new SetPreviousDirection(this, (circlev ^ endv).Normalized()));
	}
}

int LineArcDrawing::number_of_steps()
{
	switch(drawing_mode)
	{
	case CircleDrawingMode:
		switch(circle_mode)
		{
		case ThreePointsCircleMode:
			return 3;
		case CentreAndRadiusCircleMode:
			return 1;
		default:
			break;
		}
		break;
	case SplineDrawingMode:
		switch(spline_mode)
		{
		case CubicSplineMode:
			return 4;
		case QuarticSplineMode:
			return 3;
		case RationalSplineMode:
			return 20;
		}
		break;
	case EllipseDrawingMode:
		return 3;
	default:
		break;
	}
	return 2;
}

int LineArcDrawing::step_to_go_to_after_last_step()
{
	switch(drawing_mode)
	{
	case LineDrawingMode:
	case ArcDrawingMode:
		return 1;
	case SplineDrawingMode:
		return 3;
	case ILineDrawingMode:
	case CircleDrawingMode:
	case EllipseDrawingMode:
	default:
		return 0;
	}
}

bool LineArcDrawing::is_a_draw_level(int level)
{
	if(drawing_mode == SplineDrawingMode && spline_mode == RationalSplineMode)
		return level>=3;
	return Drawing::is_a_draw_level(level);
}

bool LineArcDrawing::is_an_add_level(int level)
{
	switch(drawing_mode)
	{
	case CircleDrawingMode:
		switch(circle_mode)
		{
		case ThreePointsCircleMode:
			return level == 2;
		case CentreAndRadiusCircleMode:
			return level == 0;
		default:
			break;
		}
		break;
	case EllipseDrawingMode:
		return level == 2;
	case SplineDrawingMode:
		switch(spline_mode)
		{
		case CubicSplineMode:
			return level == 3;
		case QuarticSplineMode:
			return level == 2;
		case RationalSplineMode:
			return level == 20;
		}
		break;
	default:
		break;
	}
	return level == 1;
}

void LineArcDrawing::AddPoint()
{
	switch(drawing_mode)
	{
	case CircleDrawingMode:
		{
		Drawing::AddPoint();
		}
		break;

	case EllipseDrawingMode:
		Drawing::AddPoint();
		break;

	case LineDrawingMode:
	case ArcDrawingMode:
		{
			// edit the end of the previous item to be the start of the arc
			// this only happens if we are drawing tangents to other objects
			// really need to fill the gap with whatever we are tangent around
			// ellipse,arc,spline or whatever
			if(TempObject() && PrevObject())
			{
				if(PrevObject())
				{
					double spos[3];
					double epos[3];
					TempObject()->GetStartPoint(spos);
					PrevObject()->GetEndPoint(epos);
					HeeksObj* tanobject = GetStartPos().m_object1;
					if(current_view_stuff->start_pos.m_type == DigitizeTangentType && tanobject)
					switch(tanobject->GetType())
					{
						case LineType:
							//((HLine*)prev_object)->B = p;
							break;
						case ArcType:
							{
								HArc* arc = new HArc(*(HArc*)tanobject);
								arc->A = make_point(spos);
								arc->B = make_point(epos);
								AddToTempObjects(arc);
							}
							break;
						case CircleType:
							{
								HArc* arc = new HArc(make_point(spos),make_point(epos),((HCircle*)tanobject)->GetCircle(),&wxGetApp().current_color);
								arc->A = make_point(spos);
								arc->B = make_point(epos);
								AddToTempObjects(arc);
							}
							break;
						}
					
				}
			} 

			Drawing::AddPoint();

		}
		break;

	default:
		Drawing::AddPoint();
		break;
	}
}

bool LineArcDrawing::calculate_item(DigitizedPoint &end){
	if(number_of_steps() > 1 && GetStartPos().m_type == DigitizeNoItemType)return false;
	if(end.m_type == DigitizeNoItemType)return false;

	switch(drawing_mode)
	{
	case LineDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != LineType){
				ClearObjectsMade();
			}
			gp_Pnt p1, p2;
			DigitizedPoint::GetLinePoints(GetStartPos(), end, p1, p2);
			if(p1.IsEqual(p2, wxGetApp().m_geom_tol))return false;
			end.m_point = p2;
			if(TempObject() == NULL){
				AddToTempObjects(new HLine(p1, p2, &wxGetApp().current_color));
			}
			else{
				((HLine*)TempObject())->A = p1;
				((HLine*)TempObject())->B = p2;
			}
		}
		return true;

	case ArcDrawingMode:
		{
			// tangential arcs
			if(TempObject() && TempObject()->GetType() != ArcType){
				ClearObjectsMade();
			}

			gp_Pnt centre;
			gp_Dir axis;
			gp_Pnt p1, p2;
			bool arc_found = DigitizedPoint::GetArcPoints(GetStartPos(), m_previous_direction_set ? (&m_previous_direction) : NULL, end, p1, p2, centre, axis);
			if(p1.IsEqual(p2, wxGetApp().m_geom_tol))return false;

			if(arc_found)
			{
				if(HArc::TangentialArc(p1, m_previous_direction, p2, centre, axis))
				{
					// arc
					gp_Circ circle(gp_Ax2(centre, axis), centre.Distance(p1));

					if(TempObject() == NULL){
						AddToTempObjects(new HArc(p1, p2, circle, &wxGetApp().current_color));
					}
					else{
						((HArc*)TempObject())->SetCircle(circle);
						((HArc*)TempObject())->A = p1;
						((HArc*)TempObject())->B = p2;
					}
				}
				else
				{
					// line
					if(TempObject() && TempObject()->GetType() != LineType){
						ClearObjectsMade();
					}
					if(TempObject()==NULL){
						AddToTempObjects(new HLine(p1, p2, &wxGetApp().current_color));
					}
					else{
						((HLine*)TempObject())->A = p1;
						((HLine*)TempObject())->B = p2;
					}
				}
			}
		}
		return true;

	case ILineDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != ILineType){
				ClearObjectsMade();
			}
			gp_Pnt p1, p2;
			DigitizedPoint::GetLinePoints(GetStartPos(), end, p1, p2);
			if(p1.IsEqual(p2, wxGetApp().m_geom_tol))return false;
			if(TempObject() == NULL){
				AddToTempObjects(new HILine(p1, p2, &wxGetApp().current_color));
			}
			else{
				((HILine*)TempObject())->A = p1;
				((HILine*)TempObject())->B = p2;
			}
		}
		return true;

	case EllipseDrawingMode:
		if(TempObject() && TempObject()->GetType() != EllipseType){
			ClearObjectsMade();
		}

		if(TempObject() == NULL)
		{
			gp_Elips elip;
			DigitizedPoint::GetEllipse(GetBeforeStartPos(), GetStartPos(), end,elip);
					
			AddToTempObjects(new HEllipse(elip, &wxGetApp().current_color));

		}
		else
		{
			gp_Elips elip;
			DigitizedPoint::GetEllipse(GetBeforeStartPos(), GetStartPos(), end,elip);
			((HEllipse*)TempObject())->SetEllipse(elip);
		}
		return true;	

	case SplineDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != SplineType){
				ClearObjectsMade();
			}

			Handle_Geom_BSplineCurve spline;
			switch(spline_mode)
			{
				case CubicSplineMode:
					DigitizedPoint::GetCubicSpline(GetBeforeBeforeStartPos(), GetBeforeStartPos(), GetStartPos(), end, spline);
					break;
				case QuarticSplineMode:
					DigitizedPoint::GetQuarticSpline(GetBeforeStartPos(), GetStartPos(), end, spline);
					break;
				case RationalSplineMode:
					DigitizedPoint::GetRationalSpline(spline_points, end, spline);
					break;
			}

			if(TempObject() == NULL){
				AddToTempObjects(new HSpline(spline, &wxGetApp().current_color));
			}
			else{
				((HSpline*)TempObject())->m_spline = spline;
			}

			return true;
		}
	case CircleDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != CircleType){
				ClearObjectsMade();
			}
			
			switch(circle_mode)
			{
			case CentreAndPointCircleMode:
				{
					gp_Pnt p1, p2, centre;
					gp_Dir axis;
					DigitizedPoint::GetArcPoints(GetStartPos(), NULL, end, p1, p2, centre, axis);
					radius_for_circle = p1.Distance(p2);

					if(TempObject() == NULL){
						AddToTempObjects(new HCircle(gp_Circ(gp_Ax2(p1, gp_Dir(0, 0, 1)), radius_for_circle), &wxGetApp().current_color));
					}
					else{
						((HCircle*)TempObject())->m_axis.SetLocation(p1);
						((HCircle*)TempObject())->m_radius = radius_for_circle;
					}
				}
				return true;

			case ThreePointsCircleMode:
				{
					gp_Circ c;
					if(DigitizedPoint::GetTangentCircle(GetBeforeStartPos(), GetStartPos(), end, c))
					{
						if(TempObject() == NULL){
							AddToTempObjects(new HCircle(c, &wxGetApp().current_color));
						}
						else{
							((HCircle*)TempObject())->SetCircle(c);
						}
					}
				}
				return true;
			case TwoPointsCircleMode:
				{
					gp_Circ c;
					if(DigitizedPoint::GetCircleBetween(GetStartPos(), end, c))
					{
						if(TempObject() == NULL){
							AddToTempObjects(new HCircle(c, &wxGetApp().current_color));
						}
						else{
							((HCircle*)TempObject())->SetCircle(c);
						}
					}
				}
				return true;
			case CentreAndRadiusCircleMode:
				{
					if(TempObject()==NULL){
						AddToTempObjects(new HCircle(gp_Circ(gp_Ax2(end.m_point, gp_Dir(0, 0, 1)), radius_for_circle), &wxGetApp().current_color));
					}
					else{
						((HCircle*)TempObject())->m_axis.SetLocation(end.m_point);
						((HCircle*)TempObject())->m_radius = radius_for_circle;
					}
				}
				return true;

			}
		}
		break;
	}

	return false;
}

HeeksObj* LineArcDrawing::GetOwnerForDrawingObjects()
{
	switch(drawing_mode)
	{
	case LineDrawingMode:
	case ArcDrawingMode:
		{
			if(m_add_to_sketch)
			{
				if(m_container == NULL)
				{
					m_container = new CSketch();
					wxGetApp().AddUndoably(m_container, NULL, NULL);
				}
				return m_container;
			}
		}
		break;
	default:
		break;
	}

	return &wxGetApp(); //Object always needs to be added somewhere
}


static wxString str_for_GetTitle;

const wxChar* LineArcDrawing::GetTitle()
{
	switch(drawing_mode)
	{
	case LineDrawingMode:
		str_for_GetTitle = wxString(_("Line drawing mode"));
		str_for_GetTitle.Append(wxString(_T(" : ")));
		if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on start point")));
		else str_for_GetTitle.Append(wxString(_("click on end of line")));
		return str_for_GetTitle;

	case ArcDrawingMode:
		str_for_GetTitle = wxString(_("Arc drawing mode"));
		str_for_GetTitle.Append(wxString(_T(" : ")));
		if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on start point")));
		else str_for_GetTitle.Append(wxString(_("click on end of arc")));
		return str_for_GetTitle;

	case ILineDrawingMode:
		str_for_GetTitle = wxString(_("Infinite line drawing"));
		str_for_GetTitle.Append(wxString(_T(" : ")));
		if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on first point")));
		else str_for_GetTitle.Append(wxString(_("click on second point")));
		return str_for_GetTitle;

	case EllipseDrawingMode:
		str_for_GetTitle = wxString(_("Ellipse drawing mode"));
		str_for_GetTitle.Append(wxString(_T(" : ")));

		str_for_GetTitle.Append(wxString(_("center and 2 points mode")));
		str_for_GetTitle.Append(wxString(_T("\n  ")));
		if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on center point")));
		else if(GetDrawStep() == 1) 
		{
			str_for_GetTitle.Append(wxString(_("click on point on ellipse")));
			str_for_GetTitle.Append(wxString(_T("\n  ")));
			str_for_GetTitle.Append(wxString(_("(colinear or orthogonal to axis)")));
		}
		else str_for_GetTitle.Append(wxString(_("click on another point on ellipse")));
		
		return str_for_GetTitle;

	case SplineDrawingMode:
		
		str_for_GetTitle = wxString(_("Spline drawing mode"));
		str_for_GetTitle.Append(wxString(_T(" : ")));

		switch(spline_mode){
			case CubicSplineMode:
				str_for_GetTitle.Append(wxString(_("cubic spline mode")));
				str_for_GetTitle.Append(wxString(_T("\n  ")));
				if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on start point")));
				else if(GetDrawStep() == 1) str_for_GetTitle.Append(wxString(_("click on end point")));
				else if(GetDrawStep() == 2) str_for_GetTitle.Append(wxString(_("click on first control point")));
				else str_for_GetTitle.Append(wxString(_("click on second control point")));
				break;
			case QuarticSplineMode:
				str_for_GetTitle.Append(wxString(_("quartic spline mode")));
				str_for_GetTitle.Append(wxString(_T("\n  ")));
				if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on start point")));
				else if(GetDrawStep() == 1) str_for_GetTitle.Append(wxString(_("click on end point")));
				else str_for_GetTitle.Append(wxString(_("click on control point")));
				break;
			case RationalSplineMode:
				str_for_GetTitle.Append(wxString(_("rational spline mode")));
				str_for_GetTitle.Append(wxString(_T("\n  ")));	
				if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on start point")));
				else if(GetDrawStep() == 1) str_for_GetTitle.Append(wxString(_("click on first control point")));
				else if(GetDrawStep() == 2) str_for_GetTitle.Append(wxString(_("click on second control point")));
				else str_for_GetTitle.Append(wxString(_("click on next control point or endpoint")));
	
				break;
		}

		return str_for_GetTitle;

	case CircleDrawingMode:
		str_for_GetTitle = wxString(_("Circle drawing mode"));
		str_for_GetTitle.Append(wxString(_T(" : ")));

		switch(circle_mode){
		case CentreAndPointCircleMode:
			{
				str_for_GetTitle.Append(wxString(_("center and point mode")));
				str_for_GetTitle.Append(wxString(_T("\n  ")));
				if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on center point")));
				else str_for_GetTitle.Append(wxString(_("click on point on circle")));
			}
			break;
		case ThreePointsCircleMode:
			{
				str_for_GetTitle.Append(wxString(_("three points mode")));
				str_for_GetTitle.Append(wxString(_T("\n  ")));
				if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on first point")));
				else if(GetDrawStep() == 1)str_for_GetTitle.Append(wxString(_("click on second point")));
				else str_for_GetTitle.Append(wxString(_("click on third point")));
			}
			break;
		case TwoPointsCircleMode:
			{
				str_for_GetTitle.Append(wxString(_("two points mode")));
				str_for_GetTitle.Append(wxString(_T("\n  ")));
				if(GetDrawStep() == 0)str_for_GetTitle.Append(wxString(_("click on first point")));
				else str_for_GetTitle.Append(wxString(_("click on second point")));
			}
			break;
		case CentreAndRadiusCircleMode:
			{
				str_for_GetTitle.Append(wxString(_("centre with radius mode")));
				str_for_GetTitle.Append(wxString(_T("\n  ")));
				str_for_GetTitle.Append(wxString(_("click on centre point")));
			}
			break;
		}
		return str_for_GetTitle;

	default:
		return _("unknown");
	}
}

void LineArcDrawing::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case 'A':
		// switch to arc drawing mode until a is released
		if(!m_A_down){
			m_A_down = true;
			m_save_drawing_mode.push_back(drawing_mode);
			drawing_mode = ArcDrawingMode;
			wxGetApp().m_frame->RefreshInputCanvas();
			RecalculateAndRedraw(wxPoint(event.GetX(), event.GetY()));
			wxGetApp().OnInputModeTitleChanged();
		}
		return;
	}

	Drawing::OnKeyDown(event);
}

void LineArcDrawing::OnKeyUp(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case 'A':
		// switch back to previous drawing mode
		if(m_save_drawing_mode.size()>0){
			drawing_mode = m_save_drawing_mode.back();
			m_save_drawing_mode.pop_back();
		}
		wxGetApp().m_frame->RefreshInputCanvas();
		RecalculateAndRedraw(wxPoint(event.GetX(), event.GetY()));
		wxGetApp().OnInputModeTitleChanged();
		m_A_down = false;
		return;
	}

	Drawing::OnKeyUp(event);
}

void LineArcDrawing::set_cursor(void){
}

static LineArcDrawing* line_drawing_for_GetProperties = NULL;

static void on_set_drawing_mode(int drawing_mode, HeeksObj* object, bool from_undo_redo)
{
	line_drawing_for_GetProperties->drawing_mode = (EnumDrawingMode)drawing_mode;
	line_drawing_for_GetProperties->m_save_drawing_mode.clear();
	wxGetApp().m_frame->RefreshProperties();
	wxGetApp().Repaint();
}

static void on_set_circle_mode(int circle_mode, HeeksObj* object, bool from_undo_redo)
{
	line_drawing_for_GetProperties->circle_mode = (EnumCircleDrawingMode)circle_mode;
	wxGetApp().m_frame->RefreshProperties();
	wxGetApp().Repaint();
}

static void on_set_circle_radius(double value, HeeksObj* object)
{
	line_drawing_for_GetProperties->radius_for_circle = value;
	HeeksConfig config;
	config.Write(_T("RadiusForCircle"), value);
}

void LineArcDrawing::GetProperties(std::list<Property *> *list){
	line_drawing_for_GetProperties = this;

	// add drawing mode
	{
		std::list< wxString > choices;
		choices.push_back ( wxString ( _("draw lines") ) );
		choices.push_back ( wxString ( _("draw tangential arcs") ) );
		choices.push_back ( wxString ( _("infinite line") ) );
		choices.push_back ( wxString ( _("draw circles") ) );
		list->push_back ( new PropertyChoice ( _("drawing mode"),  choices, drawing_mode, NULL, on_set_drawing_mode ) );
	}

	switch(drawing_mode)
	{
	case LineDrawingMode:
		{
			list->push_back(new PropertyString(_("(press 'a' for arcs)"), _T(""), NULL));
		}
		break;

	case CircleDrawingMode:
		{
			std::list< wxString > choices;
			choices.push_back ( wxString ( _("centre and point") ) );
			choices.push_back ( wxString ( _("three points") ) );
			choices.push_back ( wxString ( _("two points") ) );
			choices.push_back ( wxString ( _("centre and radius") ) );
			list->push_back ( new PropertyChoice ( _("circle mode"),  choices, circle_mode, NULL, on_set_circle_mode ) );
			list->push_back(new PropertyLength(_("radius"), radius_for_circle, NULL, on_set_circle_radius));
		}
		break;

	default:
		break;
	}

	Drawing::GetProperties(list);
}

void LineArcDrawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	Drawing::GetTools(f_list, p);
}

bool LineArcDrawing::OnModeChange(void){
	// on start of drawing mode
	if(!Drawing::OnModeChange())return false;
	if(m_container)m_container = NULL;

	HeeksConfig config;
	config.Read(_T("RadiusForCircle"), &radius_for_circle, 5.0);

	ClearPrevObject();
	m_previous_direction_set = false;

	return true;
}

void LineArcDrawing::set_draw_step_not_undoable(int s)
{
	Drawing::set_draw_step_not_undoable(s);
	if(drawing_mode == SplineDrawingMode && spline_mode == RationalSplineMode)
	{
		spline_points.push_back(GetStartPos());
	}
}
