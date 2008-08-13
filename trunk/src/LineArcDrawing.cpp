// LineArcDrawing.cpp

#include "stdafx.h"

#include "LineArcDrawing.h"
#include "../interface/HeeksObj.h"
#include "../interface/Tool.h"
#include "HLine.h"
#include "HArc.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyString.h"
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"
#include "LineArcCollection.h"

wxCursor LineArcDrawing::m_cursor_start;
wxCursor LineArcDrawing::m_cursor_end;

LineArcDrawing line_strip;

LineArcDrawing::LineArcDrawing(void){
	temp_object = NULL;
	m_previous_direction = gp_Vec(1, 0, 0);
	drawing_mode = 0;
	m_A_down = false;
	m_container = NULL;
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
		// tangential arcs
		if(temp_object && temp_object->GetType() != ArcType){
			delete temp_object;
			temp_object = NULL;
			temp_object_in_list.clear();
		}

		gp_Pnt centre;
		gp_Vec axis;
		gp_Pnt startp = GetStartPos();
		if(HArc::TangentialArc(startp, m_previous_direction, end, centre, axis))
		{
			// arc
			gp_Circ circle(gp_Ax2(centre, axis), centre.Distance(GetStartPos()));

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
		else
		{
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
	}
}

HeeksObj* LineArcDrawing::GetOwnerForDrawingObjects()
{
	if(m_container == NULL)
	{
		m_container = new CLineArcCollection;
		wxGetApp().AddUndoably(m_container, NULL, NULL);
	}
	return m_container;
}

void LineArcDrawing::clear_drawing_objects()
{
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
			drawing_mode = 1;
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
	list->push_back(new PropertyString("(press 'a' for arcs)", ""));
	wxGetApp().m_digitizing->GetProperties(list); // x, y, z
}

void LineArcDrawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	Drawing::GetTools(f_list, p);
}

void LineArcDrawing::GetOptions(std::list<Property *> *list){
	wxGetApp().m_select_mode->GetOptions(list);
}

bool LineArcDrawing::OnModeChange(void){
	if(!__super::OnModeChange())return false;
	drawing_mode = 0;
	if(m_container)m_container = NULL;

	return true;
}
