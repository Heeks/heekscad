// LineArcDrawing.cpp

#include "stdafx.h"

#include "LineArcDrawing.h"
#include "../interface/HeeksObj.h"
#include "../interface/Tool.h"
#include "HLine.h"
#include "HArc.h"
#include "HILine.h"
#include "HCircle.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyString.h"
#include "../interface/PropertyDouble.h"
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"
#include "Sketch.h"
#include "GraphicsCanvas.h"

wxCursor LineArcDrawing::m_cursor_start;
wxCursor LineArcDrawing::m_cursor_end;

LineArcDrawing line_strip;

LineArcDrawing::LineArcDrawing(void){
	temp_object = NULL;
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

class SetPreviousDirection:public Tool{
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
	const wxChar* GetTitle(){return _T("set previous direction");}
	void Run()
	{
		drawing->m_previous_direction = new_direction;
		drawing->m_previous_direction_set = true;
	}
	void RollBack()
	{
		if(old_previous_direction_set)drawing->m_previous_direction = old_direction;
		drawing->m_previous_direction_set = old_previous_direction_set;
	}
	bool Undoable(){return true;}
};

void LineArcDrawing::set_previous_direction(){
	if(temp_object == NULL)return;

	if(temp_object->GetType() == LineType){
		double s[3], e[3];
		if(temp_object->GetStartPoint(s) && temp_object->GetEndPoint(e))
		{
			wxGetApp().DoToolUndoably(new SetPreviousDirection(this, make_vector(make_point(s), make_point(e))));
		}
	}
	else if(temp_object->GetType() == ArcType){
		gp_Vec circlev(((HArc*)temp_object)->m_circle.Axis().Direction());
		gp_Vec endv(((HArc*)temp_object)->m_circle.Location(), ((HArc*)temp_object)->B);
		wxGetApp().DoToolUndoably(new SetPreviousDirection(this, (circlev ^ endv).Normalized()));
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
		}
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
	case ILineDrawingMode:
	case CircleDrawingMode:
	default:
		return 0;
	}
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
		}
	}
	return level == 1;
}

void LineArcDrawing::AddPoint()
{
	switch(drawing_mode)
	{
	case CircleDrawingMode:
		{
#if 0
			// kill focus on control being typed into
			wxGetApp().m_frame->m_input_canvas->DeselectProperties();
			wxGetApp().ProcessPendingEvents();

			// add a circle
			HCircle* new_object = new HCircle(gp_Circ(gp_Ax2(wxGetApp().m_digitizing->digitized_point.m_point, gp_Dir(0, 0, 1)), radius_for_circle), &wxGetApp().construction_color);
			wxGetApp().AddUndoably(new_object, GetOwnerForDrawingObjects(), NULL);
			std::list<HeeksObj*> list;
			list.push_back(new_object);
			wxGetApp().m_frame->m_graphics->DrawObjectsOnFront(list, true);
			m_getting_position = false;
			m_inhibit_coordinate_change = false;
#endif
		__super::AddPoint();
		}
		break;

	case LineDrawingMode:
	case ArcDrawingMode:
		{
			// edit the end of the previous item to be the start of the arc
			HeeksObj* save_temp_object = temp_object;

			if(temp_object && prev_object_in_list.size() > 0)
			{
				HeeksObj* prev_object = prev_object_in_list.front();
				if(prev_object)
				{
					double pos[3];
					if(temp_object->GetStartPoint(pos)){
						gp_Pnt p = make_point(pos);
						switch(prev_object->GetType())
						{
						case LineType:
							((HLine*)prev_object)->B = p;
							break;
						case ArcType:
							((HArc*)prev_object)->B = p;
						}
					}
				}
			}

			__super::AddPoint();

			// and move the point
			if(current_view_stuff->start_pos.m_type == DigitizeTangentType && save_temp_object)
			{
				double pos[3];
				if(save_temp_object->GetEndPoint(pos)){
					gp_Pnt p = make_point(pos);
					current_view_stuff->start_pos.m_point = p;
				}
			}
		}
		break;

	default:
		__super::AddPoint();
		break;
	}
}

bool LineArcDrawing::calculate_item(DigitizedPoint &end){
	if(GetStartPos().m_type == DigitizeNoItemType)return false;
	if(end.m_type == DigitizeNoItemType)return false;

	switch(drawing_mode)
	{
	case LineDrawingMode:
		{
			if(temp_object && temp_object->GetType() != LineType){
				delete temp_object;
				temp_object = NULL;
				temp_object_in_list.clear();
			}
			gp_Pnt p1, p2;
			DigitizedPoint::GetLinePoints(GetStartPos(), end, p1, p2);
			if(p1.IsEqual(p2, wxGetApp().m_geom_tol))return false;
			end.m_point = p2;
			if(!temp_object){
				temp_object = new HLine(p1, p2, &wxGetApp().current_color);
				if(temp_object)temp_object_in_list.push_back(temp_object);
			}
			else{
				((HLine*)temp_object)->A = p1;
				((HLine*)temp_object)->B = p2;
			}
		}
		return true;

	case ArcDrawingMode:
		{
			// tangential arcs
			if(temp_object && temp_object->GetType() != ArcType){
				delete temp_object;
				temp_object = NULL;
				temp_object_in_list.clear();
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

					if(!temp_object){
						temp_object = new HArc(p1, p2, circle, &wxGetApp().current_color);
						if(temp_object)temp_object_in_list.push_back(temp_object);
					}
					else{
						((HArc*)temp_object)->m_circle = circle;
						((HArc*)temp_object)->A = p1;
						((HArc*)temp_object)->B = p2;
					}
				}
				else
				{
					// line
					if(temp_object && temp_object->GetType() != LineType){
						delete temp_object;
						temp_object = NULL;
						temp_object_in_list.clear();
					}
					if(!temp_object){
						temp_object = new HLine(p1, p2, &wxGetApp().current_color);
						if(temp_object)temp_object_in_list.push_back(temp_object);
					}
					else{
						((HLine*)temp_object)->A = p1;
						((HLine*)temp_object)->B = p2;
					}
				}
			}
		}
		return true;

	case ILineDrawingMode:
		{
			if(temp_object && temp_object->GetType() != ILineType){
				delete temp_object;
				temp_object = NULL;
				temp_object_in_list.clear();
			}
			gp_Pnt p1, p2;
			DigitizedPoint::GetLinePoints(GetStartPos(), end, p1, p2);
			if(p1.IsEqual(p2, wxGetApp().m_geom_tol))return false;
			if(!temp_object){
				temp_object = new HILine(p1, p2, &wxGetApp().construction_color);
				if(temp_object)temp_object_in_list.push_back(temp_object);
			}
			else{
				((HILine*)temp_object)->A = p1;
				((HILine*)temp_object)->B = p2;
			}
		}
		return true;

	case CircleDrawingMode:
		{
			if(temp_object && temp_object->GetType() != CircleType){
				delete temp_object;
				temp_object = NULL;
				temp_object_in_list.clear();
			}
			
			switch(circle_mode)
			{
			case CentreAndPointCircleMode:
				{
					gp_Pnt p1, p2, centre;
					gp_Dir axis;
					DigitizedPoint::GetArcPoints(GetStartPos(), NULL, end, p1, p2, centre, axis);
					radius_for_circle = p1.Distance(p2);

					if(!temp_object){
						temp_object = new HCircle(gp_Circ(gp_Ax2(p1, gp_Dir(0, 0, 1)), radius_for_circle), &wxGetApp().construction_color);
						if(temp_object)temp_object_in_list.push_back(temp_object);
					}
					else{
						((HCircle*)temp_object)->m_circle.SetLocation(p1);
						((HCircle*)temp_object)->m_circle.SetRadius(radius_for_circle);
					}
				}
				return true;

			case ThreePointsCircleMode:
				{
					gp_Circ c;
					if(DigitizedPoint::GetTangentCircle(GetBeforeStartPos(), GetStartPos(), end, c))
					{
						if(!temp_object){
							temp_object = new HCircle(c, &wxGetApp().construction_color);
							if(temp_object)temp_object_in_list.push_back(temp_object);
						}
						else{
							((HCircle*)temp_object)->m_circle = c;
						}
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
					m_container = (ObjList*)new CSketch;
					wxGetApp().AddUndoably(m_container, NULL, NULL);
				}
				return m_container;
			}
		}
		break;
	}
	return NULL;
}

void LineArcDrawing::clear_drawing_objects(int mode)
{
	if(mode == 1)
	{
		prev_object_in_list = temp_object_in_list;
	}
	if(temp_object && mode == 2)delete temp_object;
	temp_object = NULL;
	temp_object_in_list.clear();
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
			wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
			RecalculateAndRedraw(wxPoint(event.GetX(), event.GetY()));
		}
		return;
	}

	__super::OnKeyDown(event);
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
		wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
		RecalculateAndRedraw(wxPoint(event.GetX(), event.GetY()));
		m_A_down = false;
		return;
	}

	__super::OnKeyUp(event);
}

void LineArcDrawing::set_cursor(void){
}

static LineArcDrawing* line_drawing_for_GetProperties = NULL;

static void on_set_drawing_mode(int drawing_mode, HeeksObj* object)
{
	line_drawing_for_GetProperties->drawing_mode = (EnumDrawingMode)drawing_mode;
	line_drawing_for_GetProperties->m_save_drawing_mode.clear();
}

static void on_set_circle_radius(double value, HeeksObj* object)
{
	line_drawing_for_GetProperties->radius_for_circle = value;
	wxGetApp().m_config->Write(_T("RadiusForCircle"), value);
}

void LineArcDrawing::GetProperties(std::list<Property *> *list){
	// add drawing mode
	std::list< wxString > choices;
	choices.push_back ( wxString ( _("draw lines") ) );
	choices.push_back ( wxString ( _("draw tangential arcs") ) );
	choices.push_back ( wxString ( _("infinite line") ) );
	choices.push_back ( wxString ( _("draw circles") ) );
	line_drawing_for_GetProperties = this;
	list->push_back ( new PropertyChoice ( _("drawing mode"),  choices, drawing_mode, NULL, on_set_drawing_mode ) );
	switch(drawing_mode)
	{
	case LineDrawingMode:
		{
			list->push_back(new PropertyString(_("(press 'a' for arcs)"), _T(""), NULL));
		}
		break;

	case CircleDrawingMode:
		{
			list->push_back(new PropertyDouble(_("radius"), radius_for_circle, NULL, on_set_circle_radius));
		}
		break;
	}

	__super::GetProperties(list);
}

void LineArcDrawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	Drawing::GetTools(f_list, p);
}

void LineArcDrawing::GetOptions(std::list<Property *> *list){
	wxGetApp().m_select_mode->GetOptions(list);
}

bool LineArcDrawing::OnModeChange(void){
	// on start of drawing mode
	if(!__super::OnModeChange())return false;
	if(m_container)m_container = NULL;

	wxGetApp().m_config->Read(_T("RadiusForCircle"), &radius_for_circle, 5.0);

	prev_object_in_list.clear();
	m_previous_direction_set = false;

	return true;
}
