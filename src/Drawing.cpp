// Drawing.cpp

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

Drawing::Drawing(void): m_inhibit_coordinate_change(false), m_getting_position(false){
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

	wxGetApp().m_frame->m_graphics->EndDrawFront();
	calculate_item(end);
	wxGetApp().m_frame->m_graphics->DrawFront();
}

void Drawing::AddPoint()
{
	// kill focus on control being typed into
	wxGetApp().m_frame->m_input_canvas->DeselectProperties();
	wxGetApp().ProcessPendingEvents();

	if(wxGetApp().m_digitizing->digitized_point.m_type == DigitizeNoItemType)return;

	wxGetApp().StartHistory(get_drawing_title());
	if(is_an_add_level(GetDrawStep())){
		calculate_item(wxGetApp().m_digitizing->digitized_point);
		before_add_item();
		const std::list<HeeksObj*>& drawing_objects = GetObjectsMade();
		wxGetApp().AddUndoably(drawing_objects, GetOwnerForDrawingObjects());
		wxGetApp().m_frame->m_graphics->DrawObjectsOnFront(drawing_objects, true);
		set_previous_direction();
		clear_drawing_objects(true);
		SetStartPosUndoable(wxGetApp().m_digitizing->digitized_point);
	}
	else{
		clear_drawing_objects();
		SetStartPosUndoable(wxGetApp().m_digitizing->digitized_point);
	}

	int next_step = GetDrawStep() + 1;
	if(next_step >= number_of_steps()){
		next_step = step_to_go_to_after_last_step();
	}
	SetDrawStepUndoable(next_step);
	wxGetApp().EndHistory();
	m_getting_position = false;
	m_inhibit_coordinate_change = false;
}

void Drawing::OnMouse( wxMouseEvent& event )
{
	bool event_used = false;

	if(LeftAndRightPressed(event, event_used))
	{
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
					button_down_point = wxPoint(event.GetX(), event.GetY());
				}
			}
			else if(event.LeftUp()){
				if(m_inhibit_coordinate_change){
					m_inhibit_coordinate_change = false;
				}
				else{
					set_digitize_plane();
					wxGetApp().m_digitizing->digitize(button_down_point);
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
					wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
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

class EndDrawing:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){wxGetApp().SetInputMode(wxGetApp().m_select_mode);}
	const wxChar* GetTitle(){return _T("Stop drawing");}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/enddraw.png"), wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const wxChar* GetToolTip(){return _T("Finish drawing");}
};
wxBitmap* EndDrawing::m_bitmap = NULL;

static EndDrawing end_drawing;

class AddPointTool:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	Drawing* m_drawing;

	AddPointTool(): m_drawing(NULL){}

	void Run()
	{
		wxGetApp().m_digitizing->digitized_point.m_type = DigitizeInputType;
		m_drawing->AddPoint();
	}
	const wxChar* GetTitle(){return _T("Add point");}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/add.png"), wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const wxChar* GetToolTip(){return _T("Add a point to drawing");}
};
wxBitmap* AddPointTool::m_bitmap = NULL;

static AddPointTool add_point;

class GetPosTool:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	Drawing* m_drawing;

	GetPosTool(): m_drawing(NULL){}

	void Run()
	{
		m_drawing->m_getting_position = true;
	}
	const wxChar* GetTitle(){return _T("Get Position");}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/pickpos.png"), wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const wxChar* GetToolTip(){return _T("Pick position without adding to the drawing");}
};
wxBitmap* GetPosTool::m_bitmap = NULL;

static GetPosTool get_pos_tool;

void Drawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	f_list->push_back(&end_drawing);
	add_point.m_drawing = this;
	f_list->push_back(&add_point);
	get_pos_tool.m_drawing = this;
	f_list->push_back(&get_pos_tool);
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
	const wxChar* GetTitle(){return _T("set_draw_step");}
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
	const wxChar* GetTitle(){return _T("set_position");}
	void Run(){drawing->set_start_pos_not_undoable(next_pos);}
	void RollBack(){drawing->set_start_pos_not_undoable(prev_pos);}
	bool Undoable(){return true;}
};

void Drawing::SetDrawStepUndoable(int s){
	wxGetApp().DoToolUndoably(new SetDrawingDrawStep(this, s));
}

void Drawing::SetStartPosUndoable(const DigitizedPoint& pos){
	wxGetApp().DoToolUndoably(new SetDrawingPosition(this, pos));
}

void Drawing::OnFrontRender(){
	if(GetDrawStep()){
		std::list<HeeksObj*>::const_iterator It;
		const std::list<HeeksObj*>& drawing_objects = GetObjectsMade();
		for(It = drawing_objects.begin(); It != drawing_objects.end(); It++){
			HeeksObj *object = *It;
			object->glCommands(false, false, true);
		}
	}

	wxGetApp().m_digitizing->OnFrontRender();
}

void Drawing::GetProperties(std::list<Property *> *list){
	wxGetApp().m_digitizing->GetProperties(list); // x, y, z
}

void Drawing::GetOptions(std::list<Property *> *list){
	wxGetApp().m_digitizing->GetOptions(list);
}
