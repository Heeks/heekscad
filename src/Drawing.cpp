// Drawing.cpp

#include "stdafx.h"

#include "Drawing.h"
#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include "../interface/Tool.h"
#include "LineArcDrawing.h"
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "HeeksFrame.h"
#include "GraphicsCanvas.h"

Drawing::Drawing(void){
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

	DigitizeType type_found = wxGetApp().m_digitizing->digitize(point);
	gp_Pnt end = wxGetApp().m_digitizing->position_found;
	if(type_found == DigitizeNoItemType)return;

	wxGetApp().m_frame->m_graphics->EndDrawFront();
	calculate_item(end);
	wxGetApp().m_frame->m_graphics->DrawFront();
}

void Drawing::OnMouse( wxMouseEvent& event )
{
	bool event_used = false;

	if(LeftAndRightPressed(event, event_used))
	{
		wxGetApp().SetInputMode(wxGetApp().m_select_mode);
	}

	if(!event_used){
		if(event.ShiftDown()){
			// do everything same as select mode
			wxGetApp().m_select_mode->OnMouse( event );
		}
		else{
			if(event.LeftDown()){
				button_down_point = wxPoint(event.GetX(), event.GetY());
			}
			else if(event.LeftUp()){
				set_digitize_plane();
				DigitizeType type_found = wxGetApp().m_digitizing->digitize(button_down_point);
				wxGetApp().StartHistory(get_drawing_title());
				if(is_an_add_level(GetDrawStep())){
					calculate_item(wxGetApp().m_digitizing->position_found);
					before_add_item();
					const std::list<HeeksObj*>& drawing_objects = GetObjectsMade();
					wxGetApp().AddUndoably(drawing_objects, GetOwnerForDrawingObjects());
					wxGetApp().m_frame->m_graphics->DrawObjectsOnFront(drawing_objects, true);
					set_previous_direction();
					clear_drawing_objects();
					if(type_found != DigitizeNoItemType)SetStartPosUndoable(wxGetApp().m_digitizing->position_found);
				}
				else if(GetDrawStep() == 0){
					clear_drawing_objects();
					SetStartPosUndoable(type_found == DigitizeNoItemType ? gp_Pnt(0, 0, 0) : wxGetApp().m_digitizing->position_found);
				}

				int next_step = GetDrawStep() + 1;
				if(next_step >= number_of_steps()){
					next_step = step_to_go_to_after_last_step();
				}
				SetDrawStepUndoable(next_step);
				wxGetApp().EndHistory();
			}
			else if(event.RightUp()){
				// do context menu same as select mode
				wxGetApp().m_select_mode->OnMouse(event);
			}
			else if(event.Moving()){
				RecalculateAndRedraw(wxPoint(event.GetX(), event.GetY()));
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
	if(!IsDrawing(wxGetApp().input_mode_object))SetDrawStepUndoable(false);
	return true;
}

static std::string global_string;

class EndDrawing:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){wxGetApp().SetInputMode(wxGetApp().m_select_mode);}
	const char* GetTitle(){return "Stop drawing";}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + "/bitmaps/enddraw.png", wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const char* GetToolTip(){return "Finish drawing";}
};
wxBitmap* EndDrawing::m_bitmap = NULL;

static EndDrawing end_drawing;

void Drawing::GetTools(std::list<Tool*> *f_list, const wxPoint *p){
	f_list->push_back(&end_drawing);
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
	int step;

public:
	SetDrawingDrawStep(Drawing *d, int s){drawing = d; step = s;}

	// Tool's virtual functions
	const char* GetTitle(){return "set_draw_step";}
	void Run(){drawing->set_draw_step_not_undoable(step);}
	void RollBack(){drawing->set_draw_step_not_undoable(!step);}
	bool Undoable(){return true;}
};

class SetDrawingPosition:public Tool{
private:
	Drawing *drawing;
	gp_Pnt prev_pos;
	gp_Pnt next_pos;

public:
	SetDrawingPosition(Drawing *d, const gp_Pnt &pos){
		drawing = d;
		prev_pos = d->GetStartPos();
		next_pos = pos;
	}

	// Tool's virtual functions
	const char* GetTitle(){return "set_position";}
	void Run(){drawing->set_start_pos_not_undoable(next_pos);}
	void RollBack(){drawing->set_start_pos_not_undoable(prev_pos);}
	bool Undoable(){return true;}
};

void Drawing::SetDrawStepUndoable(int s){
	wxGetApp().DoToolUndoably(new SetDrawingDrawStep(this, s));
}

void Drawing::SetStartPosUndoable(const gp_Pnt& pos){
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

void Drawing::GetOptions(std::list<Property *> *list){
	wxGetApp().m_digitizing->GetOptions(list);
}
