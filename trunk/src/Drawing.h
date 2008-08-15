// Drawing.h

#pragma once

#include "../interface/InputMode.h"
#include "../interface/LeftAndRight.h"

class HeeksObj;

class ViewSpecific{
public:
	int view;
	int draw_step;
	gp_Pnt start_pos;

	ViewSpecific(int v){
		view = v;
		draw_step = 0;
	}
};

class Drawing: public CInputMode, CLeftAndRight{
protected:
	std::map<int, ViewSpecific*> view_map;
	ViewSpecific *current_view_stuff;
	ViewSpecific *null_view;
	wxPoint button_down_point;

	// Drawing's virtual functions
	virtual void set_digitize_plane(){}
	virtual void calculate_item(const gp_Pnt &end){}
	virtual void before_add_item(){}
	virtual const std::list<HeeksObj*>& GetObjectsMade()const = 0;
	virtual void clear_drawing_objects(){}
	virtual void set_previous_direction(){}
	virtual int number_of_steps(){return 2;}
	virtual int step_to_go_to_after_last_step(){return 0;}
	virtual bool is_an_add_level(int level){return false;}
	virtual HeeksObj* GetOwnerForDrawingObjects(){return NULL;}

	void SetView(int);
	int GetView();
	void RecalculateAndRedraw(const wxPoint& point);

public:
	bool m_getting_position;
	bool m_inhibit_coordinate_change; // so that user can type into properties

	Drawing(void);
	virtual ~Drawing(void);

	// InputMode's virtual functions
	void OnMouse( wxMouseEvent& event );
	void OnKeyDown(wxKeyEvent& event);
	bool OnModeChange(void);
	void GetProperties(std::list<Property *> *list);
	void GetTools(std::list<Tool*> *f_list, const wxPoint *p);
	void OnFrontRender();
	void GetOptions(std::list<Property *> *list);

	// Drawing's virtual functions
	virtual const char* get_drawing_title(){return "Drawing";}

	int GetDrawStep()const{return current_view_stuff->draw_step;}
	void SetDrawStepUndoable(int s);
	const gp_Pnt& GetStartPos()const{return current_view_stuff->start_pos;}
	void SetStartPosUndoable(const gp_Pnt& pos);
	bool IsDrawing(CInputMode* i);
	void AddPoint();

	void set_draw_step_not_undoable(int s){current_view_stuff->draw_step = s;}
	void set_start_pos_not_undoable(const gp_Pnt& pos){current_view_stuff->start_pos = pos;}
};

