// LineArcDrawing.cpp

#include "stdafx.h"

#include "LineArcDrawing.h"
#include "../interface/HeeksObj.h"
#include "../interface/Tool.h"
#include "HLine.h"
#include "HArc.h"
#include "../interface/PropertyChoice.h"
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "HeeksFrame.h"
#include "OptionsCanvas.h"

wxCursor LineArcDrawing::m_cursor_start;
wxCursor LineArcDrawing::m_cursor_end;

LineArcDrawing line_strip;

LineArcDrawing::LineArcDrawing(void){
	temp_object = NULL;
	m_previous_direction = gp_Vec(1, 0, 0);
	drawing_mode = 0;
	m_A_down = false;
}

LineArcDrawing::~LineArcDrawing(void){
}

void LineArcDrawing::set_previous_direction(){
	if(temp_object == NULL)return;

	if(temp_object->GetType() == LineType){
		double s[3], e[3];
		if(temp_object->GetStartPoint(s) && temp_object->GetEndPoint(e))m_previous_direction = make_vector(make_point(s), make_point(e));
	}
	else if(temp_object->GetType() == ArcType){
		gp_Vec circlev(((HArc*)temp_object)->m_circle.Axis().Direction());
		gp_Vec endv(((HArc*)temp_object)->m_circle.Location(), ((HArc*)temp_object)->B);
		m_previous_direction = (circlev ^ endv).Normalized();
		if(!(((HArc*)temp_object)->m_dir))m_previous_direction = m_previous_direction * -1;
	}
}

void LineArcDrawing::calculate_item(const gp_Pnt &end){
	if(drawing_mode == 0){
		if(temp_object && temp_object->GetType() != LineType){
			delete temp_object;
			temp_object = NULL;
			temp_object_in_list.clear();
		}
		if(!temp_object){
			HeeksColor c(0);
			temp_object = new HLine(GetStartPos(), end, &wxGetApp().current_color);
			if(temp_object)temp_object_in_list.push_back(temp_object);
		}
		else{
			((HLine*)temp_object)->A = GetStartPos();
			((HLine*)temp_object)->B = end;
		}
	}
	else{
		if(temp_object && temp_object->GetType() != ArcType){
			delete temp_object;
			temp_object = NULL;
			temp_object_in_list.clear();
		}

		std::list<gp_Pnt> rl;
		gp_Pnt startp = GetStartPos();
		gp_Dir direction_for_circle;
		if(startp.Distance(end) > 0.0000000001){
			gp_Vec v1(GetStartPos(), end);
			gp_Pnt halfway(GetStartPos().XYZ() + v1.XYZ() * 0.5);
			gp_Pln pl1(halfway, v1);
			gp_Pln pl2(GetStartPos(), m_previous_direction);
			gp_Lin plane_line;
			intersect(pl1, pl2, plane_line);
			direction_for_circle = -(plane_line.Direction());
			gp_Lin l1(halfway, v1);
			intersect(plane_line, l1, rl);
		}

		if(rl.size() == 0){
			// line
			if(temp_object && temp_object->GetType() != LineType){
				delete temp_object;
				temp_object = NULL;
				temp_object_in_list.clear();
			}
			if(!temp_object){
				HeeksColor c(0);
				temp_object = new HLine(GetStartPos(), end, &wxGetApp().current_color);
				if(temp_object)temp_object_in_list.push_back(temp_object);
			}
			else{
				((HLine*)temp_object)->A = GetStartPos();
				((HLine*)temp_object)->B = end;
			}
		}
		else{
			// arc
			gp_Circ circle(gp_Ax2(rl.front(), direction_for_circle), rl.front().Distance(GetStartPos()));

			if(!temp_object){
				HeeksColor c(0);
				temp_object = new HArc(GetStartPos(), end, circle, &wxGetApp().current_color);
				if(temp_object)temp_object_in_list.push_back(temp_object);
			}
			else{
				((HArc*)temp_object)->m_circle = circle;
				((HArc*)temp_object)->A = GetStartPos();
				((HArc*)temp_object)->B = end;
			}
		}
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
			drawing_mode = 1;
			wxGetApp().m_frame->m_options->RefreshByRemovingAndAddingAll();
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
		wxGetApp().m_frame->m_options->RefreshByRemovingAndAddingAll();
		RecalculateAndRedraw(wxPoint(event.GetX(), event.GetY()));
		m_A_down = false;
		return;
	}

	__super::OnKeyUp(event);
}

void LineArcDrawing::set_cursor(void){
}

void LineArcDrawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	Drawing::GetTools(f_list, p);
}

LineArcDrawing* line_drawing_for_GetProperties = NULL;

void on_set_drawing_mode(int drawing_mode)
{
	line_drawing_for_GetProperties->drawing_mode = drawing_mode;
	line_drawing_for_GetProperties->m_save_drawing_mode.clear();
}

void LineArcDrawing::GetProperties(std::list<Property *> *list){
	// add drawing mode
	std::list< std::string > choices;
	choices.push_back ( std::string ( "draw lines" ) );
	choices.push_back ( std::string ( "draw tangential arcs" ) );
	line_drawing_for_GetProperties = this;
	list->push_back ( new PropertyChoice ( "drawing mode",  choices, drawing_mode, on_set_drawing_mode ) );

	wxGetApp().m_select_mode->GetSharedProperties(list);
	wxGetApp().m_digitizing->GetSharedProperties(list);
}

bool LineArcDrawing::OnModeChange(void){
	if(!__super::OnModeChange())return false;
	drawing_mode = 0;
	return true;
}
