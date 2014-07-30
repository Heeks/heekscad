// TreeCanvas.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "TreeCanvas.h"
#include "HeeksFrame.h"
#include "wxImageLoader.h"
#include "../interface/MarkedObject.h"

BEGIN_EVENT_TABLE(CTreeCanvas, wxScrolledWindow)
    EVT_PAINT(CTreeCanvas::OnPaint)
    EVT_MOUSE_EVENTS(CTreeCanvas::OnMouse)
    EVT_MENU_RANGE(ID_FIRST_POP_UP_MENU_TOOL, ID_FIRST_POP_UP_MENU_TOOL + 1000, CTreeCanvas::OnMenuEvent)
	EVT_KEY_DOWN(CTreeCanvas::OnKeyDown)
	EVT_KEY_UP(CTreeCanvas::OnKeyUp)
	EVT_CHAR(CTreeCanvas::OnCharEvent)
END_EVENT_TABLE()

static wxBitmap* bmp_branch_plus = NULL;
static wxBitmap* bmp_branch_minus = NULL;
static wxBitmap* bmp_branch_end_plus = NULL;
static wxBitmap* bmp_branch_end_minus = NULL;
static wxBitmap* bmp_branch_split = NULL;
static wxBitmap* bmp_branch_end = NULL;
static wxBitmap* bmp_plus = NULL;
static wxBitmap* bmp_minus = NULL;
static wxBitmap* bmp_branch_trunk = NULL;

CTreeCanvas::CTreeCanvas(wxWindow* parent)
        : wxScrolledWindow(parent),m_frozen(false), m_refresh_wanted_on_thaw(false),
		width(0), height(0), textureWidth(0), textureHeight(0), m_dragging(false), m_waiting_until_left_up(false), m_xpos(0), m_ypos(0), m_max_xpos(0)
{
	wxGetApp().RegisterObserver(this);

	bmp_branch_plus = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/branch_plus.png")));
	bmp_branch_minus = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/branch_minus.png")));
	bmp_branch_end_plus = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/branch_end_plus.png")));
	bmp_branch_end_minus = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/branch_end_minus.png")));
	bmp_branch_split = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/branch_split.png")));
	bmp_branch_end = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/branch_end.png")));
	bmp_plus = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/plus.png")));
	bmp_minus = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/minus.png")));
	bmp_branch_trunk = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/branch_trunk.png")));
    SetScrollRate( 10, 10 );
    SetVirtualSize( 92, 97 );
}

CTreeCanvas::~CTreeCanvas()
{
	wxGetApp().RemoveObserver(this);
}

wxPaintDC* CTreeCanvas::m_dc = NULL;

void CTreeCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    /* must always be here */
    wxPaintDC dc(this);
    PrepareDC(dc);

	m_dc = &dc;

	// render everything
	Render();

	m_dc = NULL;
}

const CTreeCanvas::CTreeButton* CTreeCanvas::HitTest( const wxPoint& pt )
{
	for(std::list<CTreeButton>::iterator It = m_tree_buttons.begin(); It != m_tree_buttons.end(); It++)
	{
		CTreeButton& b = *It;
		wxPoint unscrolled_pt = this->CalcUnscrolledPosition(pt);
		if(b.rect.Contains(unscrolled_pt)) //if(b.rect.Inside(pt))
		{
			return &b;
		}
	}
	return NULL;
}

static HeeksObj* clicked_object = NULL;

void CTreeCanvas::OnMouse( wxMouseEvent& event )
{
	if(wxGetApp().m_property_grid_validation)return;

	if(event.Entering()){
		SetFocus(); // so middle wheel works
	}

	if(event.LeftDown())
	{
		const CTreeButton* button = HitTest(event.GetPosition());

		if(button)
		{
			switch(button->type)
			{
			case ButtonTypePlus:
			case ButtonTypeMinus:
				SetExpanded(button->obj, button->type == 0);
				SetVirtualSize(GetRenderSize());
				this->Refresh();
				break;

			case ButtonTypeLabelBefore:
			case ButtonTypeLabel:
			default:
				OnLabelLeftDown(button->obj, event);
				clicked_object = button->obj;
				break;
			}
		}
		else
		{
			wxGetApp().m_marked_list->Clear(true);
		}

		m_button_down_point = event.GetPosition();
	}

	if(event.LeftUp())
	{
		if(m_dragging)
		{
			m_dragging = false;

			// find the object to drop on to
			const CTreeButton* button = HitTest(event.GetPosition());

			if(button == NULL || !wxGetApp().m_marked_list->ObjectMarked(button->obj)) // can only drop on to an item other than one of the items being dragged
			{
				// test drop possible
				bool drag_possible = true;

				HeeksObj* add_to = &wxGetApp();
				if(button && button->paste_into)add_to = button->paste_into;
				for(std::list<HeeksObj*>::iterator It = m_dragged_list.begin(); It != m_dragged_list.end(); It++)
				{
					HeeksObj* object = *It;

					if(!add_to->CanAdd(object) || !object->CanAddTo(add_to))
					{
						drag_possible = false;
						break;
					}
				}

				if(drag_possible)
				{
					wxGetApp().CreateUndoPoint();

					// cut the objects
					wxGetApp().Remove(m_dragged_list);

					// paste the objects
					for(std::list<HeeksObj*>::iterator It = m_dragged_list.begin(); It != m_dragged_list.end(); It++)
					{
						HeeksObj* object = *It;
						{
							if(object->OneOfAKind())
							{
								bool one_found = false;
								for(HeeksObj* child = add_to->GetFirstChild(); child; child = add_to->GetNextChild())
								{
									if(child->GetType() == object->GetType())
									{
										child->CopyFrom(object);
										one_found = true;
										break;
									}
								}
								if(!one_found)
								{
									add_to->Add(object, button ? button->paste_before : NULL);
								}
							}
							else
							{
								add_to->Add(object, button ? button->paste_before : NULL);
							}
						}
					}
					wxGetApp().Changed();
				}
				else
				{
					Refresh();
				}
			}
			else
			{
				Refresh();
			}
		}
		else
		{
			if(m_waiting_until_left_up)
			{
				wxGetApp().m_marked_list->Clear(false);
				wxGetApp().m_marked_list->Add(clicked_object, true);
			}
		}
		m_waiting_until_left_up = false;
	}

	if(event.RightDown())
	{
		const CTreeButton* button = HitTest(event.GetPosition());
		clicked_object = NULL;
		if(button && (button->type == ButtonTypeLabelBefore || button->type == ButtonTypeLabel))
		{
			clicked_object = button->obj;
			OnLabelRightDown(button->obj, event);
		}
	}

	if(event.RightUp())
	{
		// do a context menu
		MarkedObjectOneOfEach marked_object(0, clicked_object, 1, 0, NULL);
		if(m_dragging)
		{
			m_dragging = false;

			// find the object to drop on to
			const CTreeButton* button = HitTest(event.GetPosition());

			if(button == NULL || !wxGetApp().m_marked_list->ObjectMarked(button->obj)) // can only drop on to an item other than one of the items being dragged
			{
				// make a Move or Copy context menu
				HeeksObj* paste_into = NULL;
				HeeksObj* paste_before = NULL;
				if(button)
				{
					paste_into = button->paste_into;
					paste_before = button->paste_before;
				}
				wxGetApp().DoMoveOrCopyDropDownMenu(this, event.GetPosition(), &marked_object, paste_into, paste_before);
			}
			else
			{
				Refresh();
			}
		}
		else
		{
			// do a standard drop down menu
			wxGetApp().DoDropDownMenu(this, event.GetPosition(), &marked_object, true, false);
		}
	}

	if(event.Dragging())
	{
		if(event.LeftIsDown() || event.RightIsDown())
		{
			if(!m_dragging && (abs(m_button_down_point.x - event.GetX())>2 || abs(m_button_down_point.y - event.GetY())>2))
			{
				m_dragging = true;
				m_dragged_list = wxGetApp().m_marked_list->list();
			}
			if(m_dragging)
			{
				m_drag_position = CalcUnscrolledPosition(event.GetPosition());
				const CTreeButton* button = HitTest(event.GetPosition());
				m_drag_paste_rect = wxRect(0, 0, 0, 0);
				if(button && button->type == ButtonTypeLabelBefore)m_drag_paste_rect = button->rect;
				Refresh();
			}
		}
	}

	if(event.LeftDClick())
	{
		const CTreeButton* button = HitTest(event.GetPosition());
		if(button)
		{
			if(button->obj)
			{
				bool(*callback)(HeeksObj*) = NULL;
				button->obj->GetOnEdit(&callback);
				if(callback)
				{
					(*callback)(button->obj);
				}
			}
		}
	}

	event.Skip();
}

void CTreeCanvas::OnKeyDown(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_ESCAPE && wxGetApp().EndSketchMode())
	{}
	else wxGetApp().input_mode_object->OnKeyDown(event);

	event.Skip();
}

void CTreeCanvas::OnCharEvent(wxKeyEvent& event)
{
	const int ControlA = 1;
	const int ControlC = 3;
	const int ControlV = 22;

	// printf("Key event is '%d'\n", event.GetKeyCode());
	switch (event.GetKeyCode())
	{
		case ControlA:
			{
				// Select all
				std::list<HeeksObj*> obj_list;
				for(HeeksObj* object = wxGetApp().GetFirstChild(); object != NULL; object = wxGetApp().GetNextChild())
				{
					if(object->GetType() != GripperType)
					{
						obj_list.push_back(object);
					} // End if - then
				} // End for
				wxGetApp().m_marked_list->Add(obj_list, true);
				wxGetApp().Repaint();
				event.Skip();
				break;
			} // End ControlA scope

		case ControlC:
			// Copy
			wxGetApp().m_marked_list->CopySelectedItems();
			wxGetApp().Repaint();
			event.Skip();
			break;

		case ControlV:
			// Paste
			wxGetApp().Paste(NULL, NULL);
			wxGetApp().Repaint();
			event.Skip();
			break;

		default:
			break;
	} // End switch
} // End OnCharEvent() method


void CTreeCanvas::OnKeyUp(wxKeyEvent& event)
{
	wxGetApp().input_mode_object->OnKeyUp(event);
	event.Skip();
}

void CTreeCanvas::OnMenuEvent(wxCommandEvent& event)
{
	wxGetApp().on_menu_event(event);
}

void CTreeCanvas::OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)
{
	SetVirtualSize(GetRenderSize());
	Refresh();
}

void CTreeCanvas::WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list)
{
	Refresh();
}

void CTreeCanvas::Clear()
{
	Refresh();
}

void CTreeCanvas::Freeze()
{
	m_frozen = true;
}

void CTreeCanvas::Thaw()
{
	m_frozen = false;
	if(m_refresh_wanted_on_thaw)
	{
		Refresh();
		m_refresh_wanted_on_thaw = false;
	}
}

void CTreeCanvas::Refresh()
{
	if(m_frozen)
	{
		m_refresh_wanted_on_thaw = true;
	}
	else
	{
		wxScrolledWindow::Refresh(false);
	}
}

void CTreeCanvas::RefreshSoon()
{
	if(m_frozen)
	{
		m_refresh_wanted_on_thaw = true;
	}
	else if(wxGetApp().m_frame->IsShown())
	{
		wxScrolledWindow::Refresh(false);
		Update();
	}
}

bool CTreeCanvas::IsExpanded(HeeksObj* object)
{
	if(object->AutoExpand())
		// assume it is expanded if it hasn't been collapsed
		return m_collapsed.find(object) == m_collapsed.end();

	// it is expanded, if it is in the m_expanded set
	return m_expanded.find(object) != m_expanded.end();
}

void CTreeCanvas::SetExpanded(HeeksObj* object, bool bExpanded)
{
	if(bExpanded)
	{
		m_expanded.insert(object);
		m_collapsed.erase(object);
	}
	else
	{
		m_expanded.erase(object);
		m_collapsed.insert(object);
	}
}

static bool render_just_for_calculation = false;
static bool render_labels = true;

void CTreeCanvas::AddPlusOrMinusButton(HeeksObj* object, bool plus)
{
	CTreeButton b;
	b.type = plus ? ButtonTypePlus : ButtonTypeMinus;
	b.rect.x = m_xpos;
	b.rect.y = m_ypos;
	b.rect.width = 16;
	b.rect.height = 18;
	b.obj = object;
	m_tree_buttons.push_back(b);
}

void CTreeCanvas::AddLabelButton(bool expanded, HeeksObj* prev_object, bool prev_object_expanded, HeeksObj* object, HeeksObj* next_object, int label_start_x, int label_end_x)
{
	CTreeButton b;
	b.rect.x = label_start_x;
	b.rect.width = label_end_x - label_start_x;
	if(prev_object && !prev_object_expanded)
	{
		b.rect.y = m_ypos - 4;
		b.rect.height = 8;
	}
	else
	{
		b.rect.y = m_ypos;
		b.rect.height = 4;
	}
	b.type = ButtonTypeLabelBefore;
	b.obj = object;
#ifdef MULTIPLE_OWNERS
	b.paste_into = object->HEEKSOBJ_OWNER;
#else
	b.paste_into = object->m_owner;
#endif
	b.paste_before = object;
	m_tree_buttons.push_back(b);

	b.type = ButtonTypeLabel;
	b.rect.y = m_ypos + 4;
	b.rect.height = 10;
	b.paste_into = object;
	b.paste_before = NULL;
	m_tree_buttons.push_back(b);

	b.rect.y += b.rect.height;
	b.rect.height = 4;
	if(next_object == NULL && !expanded)
	{
		b.type = ButtonTypeLabelBefore;
#ifdef MULTIPLE_OWNERS
		b.paste_into = object->HEEKSOBJ_OWNER;
#else
		b.paste_into = object->m_owner;
#endif
		b.paste_before = next_object;
		m_tree_buttons.push_back(b);
	}
}

void CTreeCanvas::OnLabelLeftDown(HeeksObj* object, wxMouseEvent& event)
{
	if(event.ShiftDown())
	{
		// mark a list of siblings
#ifdef MULTIPLE_OWNERS
		HeeksObj* parent = object->HEEKSOBJ_OWNER;
#else
		HeeksObj* parent = object->m_owner;
#endif
		std::set<HeeksObj*> sibling_set;
		std::list<HeeksObj*> sibling_list;
		for(HeeksObj* sibling = parent->GetFirstChild(); sibling; sibling = parent->GetNextChild())
		{
			sibling_set.insert(sibling);
			sibling_list.push_back(sibling);
		}
		// find most recently marked sibling
		std::list<HeeksObj*> &marked = wxGetApp().m_marked_list->list();
		HeeksObj* recently_marked_sibling = NULL;
		bool recent_first = false;
		for(std::list<HeeksObj*>::reverse_iterator It = marked.rbegin(); It != marked.rend(); It++)
		{
			if(*It == object)recent_first = true;
			if(sibling_set.find(*It) != sibling_set.end())
			{
				recently_marked_sibling = *It;
				break;
			}
		}

		if(recently_marked_sibling)
		{
			if(!event.ControlDown())
			{
				wxGetApp().m_marked_list->Clear(false);
			}

			bool marking = false;
			std::list<HeeksObj*> list_to_mark;
			bool finish_marking = false;
			for(std::list<HeeksObj*>::iterator It = sibling_list.begin(); !finish_marking && It != sibling_list.end(); It++)
			{
				HeeksObj* sibling = *It;
				if(sibling == object || sibling == recently_marked_sibling)
				{
					if(marking)finish_marking = true;
					else marking = true;
				}

				if(marking)
				{
					list_to_mark.push_back(sibling);
				}
			}

			wxGetApp().m_marked_list->Add(list_to_mark, true);
		}
		else
		{
			if(event.ControlDown())
			{
				if(wxGetApp().m_marked_list->ObjectMarked(object))
				{
					wxGetApp().m_marked_list->Remove(object, true);
				}
				else{
					wxGetApp().m_marked_list->Add(object, true);
				}
			}
			else
			{
				if(wxGetApp().m_marked_list->ObjectMarked(object))
				{
					m_waiting_until_left_up = true;
				}
				else
				{
					wxGetApp().m_marked_list->Clear(false);
					wxGetApp().m_marked_list->Add(object, true);
				}
			}
		}
	}
	else
	{
		// shift not down
		if(event.ControlDown())
		{
			if(wxGetApp().m_marked_list->ObjectMarked(object))
			{
				wxGetApp().m_marked_list->Remove(object, true);
			}
			else{
				wxGetApp().m_marked_list->Add(object, true);
			}
		}
		else
		{
			if(wxGetApp().m_marked_list->ObjectMarked(object))
			{
				m_waiting_until_left_up = true;
			}
			else
			{
				wxGetApp().m_marked_list->Clear(false);
				wxGetApp().m_marked_list->Add(object, true);
			}
		}
	}
}

void CTreeCanvas::OnLabelRightDown(HeeksObj* object, wxMouseEvent& event)
{
	if(!wxGetApp().m_marked_list->ObjectMarked(object))
	{
		wxGetApp().m_marked_list->Clear(false);
		wxGetApp().m_marked_list->Add(object, true);
	}
}

void CTreeCanvas::RenderBranchIcon(HeeksObj* object, HeeksObj* next_object, bool expanded, int level)
{
	int num_children = object->GetNumChildren();
	if(num_children > 0)
	{
		if(level)
		{
			// with branches
			if(next_object)
			{
				// not at end
				if(expanded)
				{
					m_dc->DrawBitmap(*bmp_branch_minus, m_xpos, m_ypos);
					if(render_labels)AddPlusOrMinusButton(object, false);
				}
				else
				{
					m_dc->DrawBitmap(*bmp_branch_plus, m_xpos, m_ypos);
					if(render_labels)AddPlusOrMinusButton(object, true);
				}
			}
			else
			{
				// not at end
				if(expanded)
				{
					m_dc->DrawBitmap(*bmp_branch_end_minus, m_xpos, m_ypos);
					if(render_labels)AddPlusOrMinusButton(object, false);
				}
				else
				{
					m_dc->DrawBitmap(*bmp_branch_end_plus, m_xpos, m_ypos);
					if(render_labels)AddPlusOrMinusButton(object, true);
				}
			}
		}
		else
		{
			// without branches
			if(expanded)
			{
				m_dc->DrawBitmap(*bmp_minus, m_xpos, m_ypos);
				if(render_labels)AddPlusOrMinusButton(object, false);
			}
			else
			{
				m_dc->DrawBitmap(*bmp_plus, m_xpos, m_ypos);
				if(render_labels)AddPlusOrMinusButton(object, true);
			}
		}
	}
	else
	{
		if(level)
		{
			// just branches
			if(next_object)m_dc->DrawBitmap(*bmp_branch_split, m_xpos, m_ypos);
			else m_dc->DrawBitmap(*bmp_branch_end, m_xpos, m_ypos);
		}
	}
}

static std::list<bool> end_child_list;

void CTreeCanvas::RenderBranchIcons(HeeksObj* object, HeeksObj* next_object, bool expanded, int level)
{
	// render initial branches
	std::list<bool>::iterator It = end_child_list.begin();
	for(int i = 0; i<level; i++, It++)
	{
		if(!render_just_for_calculation && i > 0)
		{
			bool end_child = *It;
			if(!end_child)m_dc->DrawBitmap(*bmp_branch_trunk, m_xpos, m_ypos);
		}
		m_xpos += 16;
	}

	// render + or -
	if(!render_just_for_calculation)RenderBranchIcon(object, next_object, expanded, level);
	m_xpos += 16;
}

void CTreeCanvas::RenderObject(bool expanded, HeeksObj* prev_object, bool prev_object_expanded, HeeksObj* object, HeeksObj* next_object, int level)
{
	int save_x = m_xpos;

	RenderBranchIcons(object, next_object, expanded, level);

	int label_start_x = m_xpos;
	// find icon info
	if(!render_just_for_calculation)m_dc->DrawBitmap(object->GetIcon(), m_xpos, m_ypos);
	m_xpos += 16;

	wxString str(object->GetShortStringOrTypeString());
	if(!render_just_for_calculation)
	{
		if(render_labels && wxGetApp().m_marked_list->ObjectMarked(object))
		{
			m_dc->SetBackgroundMode(wxSOLID);
			m_dc->SetTextBackground(*wxBLUE);
			m_dc->SetTextForeground(*wxWHITE);
		}
		else
		{
			m_dc->SetBackgroundMode(wxTRANSPARENT);
			m_dc->SetTextForeground(*wxBLACK);
		}
		m_dc->DrawText(str, m_xpos, m_ypos);
	}
	int text_width = 0;
	if(render_just_for_calculation || !render_labels)
	{
		// just make a guess, we don't have a valid m_dc
		text_width = 10 * str.Len();
	}
	else
	{
		wxSize text_size = m_dc->GetTextExtent(str);
		text_width = text_size.GetWidth();
	}
	int label_end_x = m_xpos + 8 + text_width;
	if(!render_just_for_calculation && render_labels)
	{
		AddLabelButton(expanded, prev_object, prev_object_expanded, object, next_object, label_start_x, label_end_x);
	}
	if(label_end_x > m_max_xpos)m_max_xpos = label_end_x;

	m_ypos += 18;

	bool end_object = (next_object == NULL);
	end_child_list.push_back(end_object);

	m_xpos = save_x;

	if(expanded)
	{
		HeeksObj* prev_child = NULL;
		bool prev_child_expanded = false;
		HeeksObj* child = object->GetFirstChild();

		while(child)
		{
			HeeksObj* next_child = object->GetNextChild();
			bool expanded = IsExpanded(child);
			RenderObject(expanded, prev_child, prev_child_expanded, child, next_child, level + 1);
			prev_child = child;
			prev_child_expanded = expanded;
			child = next_child;
		}
	}

	end_child_list.pop_back();
}

void CTreeCanvas::Render(bool just_for_calculation)
{
	render_just_for_calculation = just_for_calculation;
	if(!just_for_calculation)
	{
#ifdef WIN32
		// draw a white background rectangle
		int w, h;
		GetClientSize(&w, &h);
		wxPoint pTopLeft = CalcUnscrolledPosition(wxPoint(0, 0));
		wxPoint pBottomRight = CalcUnscrolledPosition(wxPoint(w, h));
		m_dc->SetBrush(wxBrush(wxT("white")));
		m_dc->SetPen(wxPen(wxT("white")));
		m_dc->DrawRectangle(wxRect(pTopLeft, pBottomRight));
#endif

		// set background
		m_dc->SetBackgroundMode(wxSOLID);
		m_dc->SetBackground(wxBrush(wxT("white"), wxSOLID));
		m_dc->Clear();

		m_tree_buttons.clear();
	}

	m_xpos = 0; // start at the left
	m_ypos = 0;//-scroll_y_pos; // start at the top
	m_max_xpos = 0;

	HeeksObj* prev_object = NULL;
	bool prev_object_expanded = NULL;
	HeeksObj* object = wxGetApp().GetFirstChild();

	while(object)
	{
		HeeksObj* next_object = wxGetApp().GetNextChild();
		bool expanded = IsExpanded(object);
		RenderObject(expanded, prev_object, prev_object_expanded, object, next_object, 0);
		prev_object = object;
		prev_object_expanded = expanded;
		object = next_object;
	}

	// draw the dragged objects
	if(m_dragging)
	{
		wxSize drag_size = GetDraggedListSize();
		m_dc->SetBrush(wxBrush(wxT("orange")));
		m_dc->SetPen(wxPen(wxT("blue")));
		m_dc->DrawRectangle(m_drag_position, drag_size);
		RenderDraggedList();
		if(m_drag_paste_rect.width > 0)
		{
			m_dc->SetPen(wxPen(wxT("black")));
			m_dc->SetBrush(wxBrush(wxT("black")));
			m_dc->DrawRectangle(m_drag_paste_rect);
		}
	}
}

wxSize CTreeCanvas::GetRenderSize()
{
	bool just_for_calculation = true;
	Render(just_for_calculation);
	return wxSize(m_max_xpos, m_ypos);
}

void CTreeCanvas::RenderDraggedList(bool just_for_calculation)
{
	render_just_for_calculation = just_for_calculation;
	render_labels = false;

	if(just_for_calculation)
	{
		m_xpos = 0;
		m_ypos = 0;
	}
	else
	{
		m_xpos = m_drag_position.x;
		m_ypos = m_drag_position.y;
	}
	m_max_xpos = 0;

	if(m_dragged_list.size() > 0)
	{
		std::list<HeeksObj*>::iterator It = m_dragged_list.begin();
		HeeksObj* prev_object = NULL;
		HeeksObj* object = *It;
		for(;It != m_dragged_list.end();)
		{
			It++;
			HeeksObj* next_object = NULL;
			if(It != m_dragged_list.end())next_object = *It;
			bool expanded = IsExpanded(object);
			RenderObject(expanded, prev_object, false, object, next_object, 0);
			prev_object = object;
			object = next_object;
		}
	}
	render_labels = true;
}

wxSize CTreeCanvas::GetDraggedListSize()
{
	bool just_for_calculation = true;
	RenderDraggedList(just_for_calculation);
	return wxSize(m_max_xpos, m_ypos);
}
