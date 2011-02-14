// TreeCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// this will be renamed from TreeCanvas to TreeCanvas, when it is finished.

// this tree view is drawn with OpenGL.

#pragma once

#include "../interface/Observer.h"

class CTreeCanvas: public wxScrolledWindow, Observer
{
private:
	bool m_LButton;
	wxPoint m_CurrentPoint;
	bool m_frozen;
	bool m_refresh_wanted_on_thaw;
    int width, height, textureWidth, textureHeight;
	std::set<HeeksObj*> m_expanded;
	std::set<HeeksObj*> m_collapsed;
	//int scroll_y_pos;
	static wxPaintDC* m_dc; // temporary for each OnPaint
	std::list<HeeksObj*> m_dragged_list;
	bool m_dragging;
	wxPoint m_button_down_point;
	wxPoint m_drag_position;
	wxRect m_drag_paste_rect;
	bool m_waiting_until_left_up;

	int m_xpos, m_ypos, m_max_xpos;
	enum TreeButtonType{ ButtonTypePlus, ButtonTypeMinus, ButtonTypeLabelBefore, ButtonTypeLabel };
	class CTreeButton{public:	TreeButtonType type; wxRect rect; HeeksObj* obj; HeeksObj* paste_into; HeeksObj* paste_before;};
	std::list<CTreeButton> m_tree_buttons;

	bool IsExpanded(HeeksObj* object);
	void SetExpanded(HeeksObj* object, bool bExpanded);
	void RenderBranchIcon(HeeksObj* object, HeeksObj* next_object, bool expanded, int level);
	void RenderBranchIcons(HeeksObj* object, HeeksObj* next_object, bool expanded, int level);
	void RenderObject(bool expanded, HeeksObj* prev_object, bool prev_object_expanded, HeeksObj* object, HeeksObj* next_object, int level);
	void Render(bool just_for_calculation = false); // drawing commands for all the objects
	const CTreeButton* HitTest( const wxPoint& pt );
	wxSize GetRenderSize();
	void AddPlusOrMinusButton(HeeksObj* object, bool plus);
	void AddLabelButton(bool expanded, HeeksObj* prev_object, bool prev_object_expanded, HeeksObj* object, HeeksObj* next_object, int label_start_x, int label_end_x);
	void OnLabelLeftDown(HeeksObj* object, wxMouseEvent& event);
	void OnLabelRightDown(HeeksObj* object, wxMouseEvent& event);
	void RenderDraggedList(bool just_for_calculation = false);
	wxSize GetDraggedListSize();

public:
	CTreeCanvas(wxWindow* parent);
    virtual ~CTreeCanvas();

    void OnPaint(wxPaintEvent& event);
    void OnMouse( wxMouseEvent& event );
	void OnMenuEvent(wxCommandEvent& WXUNUSED(event));
	void OnCharEvent(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);

	// Observer's virtual functions
	void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj*>* added_list, const std::list<HeeksObj*>* removed_list);
	void Clear();
	void Freeze();
	void Thaw();
	void Refresh();
	void RefreshSoon(); // for dragging the view, for example

private:
    DECLARE_EVENT_TABLE()
};
