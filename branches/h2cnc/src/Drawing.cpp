// Drawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "Drawing.h"
#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include "../interface/Tool.h"
#include "../interface/Property.h"
#include "LineArcDrawing.h"
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"
#include "InputModeCanvas.h"
#include "PropertiesCanvas.h"

Drawing::Drawing(void): m_getting_position(false), m_inhibit_coordinate_change(false){
	null_view = new ViewSpecific(0);
	SetView(0);
}

Drawing::~Drawing(void){
	delete null_view;

	std::map<int, ViewSpecific*>::iterator It;
	for(It = view_map.begin(); It != view_map.end(); It++){
		ViewSpecific *view = It->second;
		delete view;
	}
}

void Drawing::RecalculateAndRedraw(const wxPoint& point)
{
	set_digitize_plane();

	DigitizedPoint end = wxGetApp().m_digitizing->digitize(point);
	if(end.m_type == DigitizeNoItemType)return;

	if(is_a_draw_level(GetDrawStep()))
	{
		if(DragDoneWithXOR())wxGetApp().m_current_viewport->EndDrawFront();
		calculate_item(end);
		if(DragDoneWithXOR())wxGetApp().m_current_viewport->DrawFront();
		else wxGetApp().Repaint(true);
	}
}

void Drawing::AddPoint()
{
	// kill focus on control being typed into
	wxGetApp().m_frame->m_input_canvas->DeselectProperties();
	wxGetApp().ProcessPendingEvents();

	if(wxGetApp().m_digitizing->digitized_point.m_type == DigitizeNoItemType)return;

	bool calculated = false;
	if(is_an_add_level(GetDrawStep())){
		calculated = calculate_item(wxGetApp().m_digitizing->digitized_point);
		if(calculated){
			before_add_item();
			const std::list<HeeksObj*>& drawing_objects = GetObjectsMade();
			((ObjList*)GetOwnerForDrawingObjects())->Add(drawing_objects);
			if(DragDoneWithXOR())wxGetApp().m_current_viewport->DrawObjectsOnFront(drawing_objects, true);
			else wxGetApp().Repaint();
			set_previous_direction();
			wxGetApp().Changed();
		}
	}
	
	clear_drawing_objects(calculated ? 1:0);
	SetStartPosUndoable(wxGetApp().m_digitizing->digitized_point);
	wxGetApp().m_digitizing->reference_point = wxGetApp().m_digitizing->digitized_point;

	int next_step = GetDrawStep() + 1;
	if(next_step >= number_of_steps()){
		next_step = step_to_go_to_after_last_step();
	}
	SetDrawStepUndoable(next_step);
	m_getting_position = false;
	m_inhibit_coordinate_change = false;
	wxGetApp().OnInputModeTitleChanged();
}

void Drawing::OnMouse( wxMouseEvent& event )
{
	bool event_used = false;

	if(LeftAndRightPressed(event, event_used))
	{
		if(DragDoneWithXOR())wxGetApp().m_current_viewport->EndDrawFront();
		clear_drawing_objects(2);
		wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	}

	if(!event_used){
		if(event.MiddleIsDown() || event.GetWheelRotation() != 0)
		{
			wxGetApp().m_select_mode->OnMouse(event);
		}
		else{
			if(event.LeftDown()){
				if(!m_inhibit_coordinate_change)
				{
					button_down_point = wxGetApp().m_digitizing->digitize(wxPoint(event.GetX(), event.GetY()));
				}
			}
			else if(event.LeftUp()){
				if(m_inhibit_coordinate_change){
					m_inhibit_coordinate_change = false;
				}
				else{
					set_digitize_plane();
					wxGetApp().m_digitizing->digitized_point = button_down_point;
					if(m_getting_position){
						m_inhibit_coordinate_change = true;
						m_getting_position = false;
					}
					else{
						AddPoint();
					}
				}
			}
			else if(event.RightUp()){
				// do context menu same as select mode
				wxGetApp().m_select_mode->OnMouse(event);
			}
			else if(event.Moving()){
				if(!m_inhibit_coordinate_change){
					RecalculateAndRedraw(wxPoint(event.GetX(), event.GetY()));
					wxGetApp().m_frame->RefreshInputCanvas();
				}
			}
		}
	}
}

void Drawing::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case WXK_F1:
	case WXK_RETURN:
	case WXK_ESCAPE:
		// end drawing mode
		clear_drawing_objects(2);
		wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	}
}

bool Drawing::IsDrawing(CInputMode* i){
	if(i == &line_strip)return true;
//	if(i == &arc_strip)return true;

	return false;
}

bool Drawing::OnModeChange(void){
	view_map.clear();
	*null_view = ViewSpecific(0);
	current_view_stuff = null_view;

	if(!IsDrawing(wxGetApp().input_mode_object))SetDrawStepUndoable(0);
	return true;
}

static Drawing* drawing_for_tools = NULL;

class EndDrawing:public Tool{
public:
	void Run(){if(drawing_for_tools->DragDoneWithXOR())wxGetApp().m_current_viewport->EndDrawFront();drawing_for_tools->clear_drawing_objects(2); wxGetApp().SetInputMode(wxGetApp().m_select_mode);}
	const wxChar* GetTitle(){return _("Stop drawing");}
	wxString BitmapPath(){return _T("enddraw");}
	const wxChar* GetToolTip(){return _("Finish drawing");}
};

static EndDrawing end_drawing;

class AddPointTool:public Tool{
public:
	Drawing* m_drawing;

	AddPointTool(): m_drawing(NULL){}

	void Run()
	{
		wxGetApp().m_digitizing->digitized_point.m_type = DigitizeInputType;
		m_drawing->AddPoint();
	}
	const wxChar* GetTitle(){return _("Add point");}
	wxString BitmapPath(){return _T("add");}
	const wxChar* GetToolTip(){return _("Add a point to drawing");}
};

static AddPointTool add_point;

class GetPosTool:public Tool{
public:
	Drawing* m_drawing;

	GetPosTool(): m_drawing(NULL){}

	void Run()
	{
		m_drawing->m_getting_position = true;
	}
	const wxChar* GetTitle(){return _("Get Position");}
	wxString BitmapPath(){return _T("pickpos");}
	const wxChar* GetToolTip(){return _("Pick position without adding to the drawing");}
};

static GetPosTool get_pos_tool;

void Drawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	drawing_for_tools = this;
	f_list->push_back(&end_drawing);
	add_point.m_drawing = this;
	f_list->push_back(&add_point);
	get_pos_tool.m_drawing = this;
	f_list->push_back(&get_pos_tool);
}


HeeksObj* Drawing::GetOwnerForDrawingObjects()
{
	if(wxGetApp().m_sketch_mode)
	{
		return wxGetApp().GetContainer();
	}
	return &wxGetApp(); //Object always needs to be added somewhere
}

void Drawing::SetView(int v){
	if(v == 0){
		current_view_stuff = null_view;
		return;
	}
	if(v == GetView())return;

	std::map<int, ViewSpecific*>::iterator FindIt;
	FindIt = view_map.find(v);
	if(FindIt == view_map.end()){
		current_view_stuff = new ViewSpecific(v);
		view_map.insert(std::pair<int, ViewSpecific*>(v, current_view_stuff));
	}
	else{
		current_view_stuff = FindIt->second;
	}
}

int Drawing::GetView(){
	return current_view_stuff->view;
}

class SetDrawingDrawStep:public Tool{
private:
	Drawing *drawing;
	int old_step;
	int step;

public:
	SetDrawingDrawStep(Drawing *d, int s){drawing = d; old_step = drawing->GetDrawStep(); step = s;}

	// Tool's virtual functions
	const wxChar* GetTitle(){return _("set_draw_step");}
	void Run(){drawing->set_draw_step_not_undoable(step);}
	void RollBack(){drawing->set_draw_step_not_undoable(old_step);}
	bool Undoable(){return true;}
};

class SetDrawingPosition:public Tool{
private:
	Drawing *drawing;
	DigitizedPoint prev_pos;
	DigitizedPoint next_pos;

public:
	SetDrawingPosition(Drawing *d, const DigitizedPoint &pos){
		drawing = d;
		prev_pos = d->GetStartPos();
		next_pos = pos;
	}

	// Tool's virtual functions
	const wxChar* GetTitle(){return _("set_position");}
	void Run(){drawing->set_start_pos_not_undoable(next_pos);}
	void RollBack(){drawing->set_start_pos_not_undoable(prev_pos);}
	bool Undoable(){return true;}
};

void Drawing::SetDrawStepUndoable(int s){
//	wxGetApp().DoToolUndoably(new SetDrawingDrawStep(this, s));
	SetDrawingDrawStep sds(this, s);
	sds.Run();
}

void Drawing::SetStartPosUndoable(const DigitizedPoint& pos){
//	wxGetApp().DoToolUndoably(new SetDrawingPosition(this, pos));
	SetDrawingPosition sdp(this, pos);
	sdp.Run();
}

void Drawing::OnFrontRender(){
	if(DragDoneWithXOR() && GetDrawStep()){
		std::list<HeeksObj*>::const_iterator It;
		const std::list<HeeksObj*>& drawing_objects = GetObjectsMade();

		HeeksObj *owner = GetOwnerForDrawingObjects();
		CSketch *sketch = dynamic_cast<CSketch*>(owner);
		glPushMatrix();
		if(sketch && sketch->m_coordinate_system)
		{
			sketch->m_coordinate_system->ApplyMatrix();
		}

		for(It = drawing_objects.begin(); It != drawing_objects.end(); It++){
			HeeksObj *object = *It;
			object->glCommands(false, false, true);
		}

		glPopMatrix();
	}

	wxGetApp().m_digitizing->OnFrontRender();
}

void Drawing::OnRender(){
	if(!DragDoneWithXOR() && GetDrawStep()){
		std::list<HeeksObj*>::const_iterator It;
		const std::list<HeeksObj*>& drawing_objects = GetObjectsMade();
		
		HeeksObj *owner = GetOwnerForDrawingObjects();
		CSketch *sketch = dynamic_cast<CSketch*>(owner);
		glPushMatrix();
		if(sketch && sketch->m_coordinate_system)
		{
			sketch->m_coordinate_system->ApplyMatrix();
		}

		for(It = drawing_objects.begin(); It != drawing_objects.end(); It++){
			HeeksObj *object = *It;
			object->glCommands(false, false, false);
		}

		glPopMatrix();
	}
}

void Drawing::GetProperties(std::list<Property *> *list){
	wxGetApp().m_digitizing->GetProperties(list); // x, y, z
}
