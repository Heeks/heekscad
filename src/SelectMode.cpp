// SelectMode.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "SelectMode.h"
#include "../interface/InputMode.h"
#include "../interface/Tool.h"
#include "../interface/MarkedObject.h"
#include "ViewPoint.h"
#include "MagDragWindow.h"
#include "MarkedList.h"
#include "DigitizeMode.h"
#include "Gripper.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"
#include "GripperSelTransform.h"
#include "CorrelationTool.h"

bool CSelectMode::m_can_grip_objects = true;

CSelectMode::CSelectMode( const bool include_similar_objects /* = false */ ){
	control_key_initially_pressed = false;
	window_box_exists = false;
	m_doing_a_main_loop = false;
	m_just_one = false;
	m_include_similar_objects = include_similar_objects;
}

const wxChar* CSelectMode::GetTitle()
{
	return m_doing_a_main_loop ? (m_prompt_when_doing_a_main_loop.c_str()):_("Select Mode");
}

static wxString str_for_GetHelpText;

const wxChar* CSelectMode::GetHelpText()
{
	str_for_GetHelpText = wxString(_("Left button for selecting objects"))
		+ _T("\n") + _("( with Ctrl key for extra objects)")
		+ _T("\n") + _("( with Shift key for similar objects)")
		+ _T("\n") + _("Drag with left button to window select")
		+ _T("\n") + _("or to move object if on an object")
		+ _T("\n") + _("Mouse wheel to zoom in and out");

	if(wxGetApp().ctrl_does_rotate){
		str_for_GetHelpText.Append(wxString(_T("\n")) + _("Middle button to pan view"));
		str_for_GetHelpText.Append(wxString(_T("\n")) + _("( with Ctrl key to rotate view )"));
	}
	else{
		str_for_GetHelpText.Append(wxString(_T("\n")) + _("Middle button to rotate view"));
		str_for_GetHelpText.Append(wxString(_T("\n")) + _("( with Ctrl key to pan view )"));
	}

	str_for_GetHelpText.Append(wxString(_T("\n")) + _("Right button for object menu"));
	str_for_GetHelpText.Append(wxString(_T("\n")) + _("See options window to hide this help"));
	str_for_GetHelpText.Append(wxString(_T("\n")) + _T("( ") + _("view options") + _T("->") + _("screen text") + _T(" )"));

	return str_for_GetHelpText;
}

static GripperSelTransform drag_object_gripper(gp_Pnt(0, 0, 0), GripperTypeTranslate,0, NULL);

void CSelectMode::OnMouse( wxMouseEvent& event )
{
	bool event_used = false;
	if(LeftAndRightPressed(event, event_used))
	{
		if(m_doing_a_main_loop){
			wxGetApp().ExitMainLoop();
		}
	}

	if(event_used)return;

	if(event.LeftDown())
	{
		button_down_point = wxPoint(event.GetX(), event.GetY());
		CurrentPoint = button_down_point;
		MarkedObjectManyOfSame marked_object;
		wxGetApp().FindMarkedObject(button_down_point, &marked_object);
		if(marked_object.m_map.size()>0)
		{
			HeeksObj* object = marked_object.GetFirstOfTopOnly();

			if (m_include_similar_objects)
			{
				// Augment the marked_object list with objects that 'look' like
				// the one selected.

				CCorrelationTool correlate(wxGetApp().m_min_correlation_factor, wxGetApp().m_max_scale_threshold, wxGetApp().m_number_of_sample_points );
				CCorrelationTool::Symbols_t similar_symbols = correlate.SimilarSymbols( CCorrelationTool::Symbol_t(object->GetType(), object->m_id ));
				CCorrelationTool::Symbols_t::const_iterator l_itSymbol;

				for (l_itSymbol = similar_symbols.begin(); l_itSymbol != similar_symbols.end(); l_itSymbol++)
				{
					HeeksObj *ob = wxGetApp().GetIDObject( l_itSymbol->first, l_itSymbol->second );
					wxGetApp().m_marked_list->Add(ob, true);
				} // End for

			} // End if - then

			while(object)
			{
				if(object->GetType() == GripperType)
				{
					wxGetApp().m_frame->m_graphics->DrawFront();
					wxGetApp().drag_gripper = (Gripper*)object;
					wxGetApp().m_digitizing->SetOnlyCoords(wxGetApp().drag_gripper, true);
					wxGetApp().m_digitizing->digitize(button_down_point);
					wxGetApp().grip_from = wxGetApp().m_digitizing->digitized_point.m_point;
					wxGetApp().grip_to = wxGetApp().grip_from;
					double from[3];
					from[0] = wxGetApp().grip_from.X();
					from[1] = wxGetApp().grip_from.Y();
					from[2] = wxGetApp().grip_from.Z();
					wxGetApp().drag_gripper->OnGripperGrabbed(wxGetApp().m_marked_list->list(), true, from);
					wxGetApp().grip_from = gp_Pnt(from[0], from[1], from[2]);
					wxGetApp().m_frame->m_graphics->EndDrawFront();
					return;
				}
				object = marked_object.Increment();
			}
		}
	}

	if(event.MiddleDown())
	{
		button_down_point = wxPoint(event.GetX(), event.GetY());
		CurrentPoint = button_down_point;
		wxGetApp().m_frame->m_graphics->StoreViewPoint();
		wxGetApp().m_frame->m_graphics->m_view_point.SetStartMousePoint(button_down_point);
	}

	if(event.LeftUp())
	{
		if(wxGetApp().drag_gripper)
		{
			double to[3], from[3];
			wxGetApp().m_digitizing->digitize(wxPoint(event.GetX(), event.GetY()));
			extract(wxGetApp().m_digitizing->digitized_point.m_point, to);
			wxGetApp().grip_to = wxGetApp().m_digitizing->digitized_point.m_point;
			extract(wxGetApp().grip_from, from);	
			wxGetApp().drag_gripper->OnGripperReleased(from, to);
			wxGetApp().m_digitizing->SetOnlyCoords(wxGetApp().drag_gripper, false);
			wxGetApp().drag_gripper = NULL;
		}
		else if(window_box_exists)
		{
			if(!event.ControlDown())wxGetApp().m_marked_list->Clear(true);
			if(window_box.width > 0){
				// only select objects which are completely within the window
				MarkedObjectManyOfSame marked_object;
				wxGetApp().m_marked_list->ObjectsInWindow(window_box, &marked_object, false);
				std::set<HeeksObj*> obj_set;
				for(HeeksObj* object = marked_object.GetFirstOfTopOnly(); object; object = marked_object.Increment())if(object->GetType() != GripperType)
					obj_set.insert(object);

				int bottom = window_box.y;
				int top = window_box.y + window_box.height;
				int height = abs(window_box.height);
				if(top < bottom)
				{
					int temp = bottom;
					bottom = top;
					top = temp;
				}

				wxRect strip_boxes[4];
				// top
				strip_boxes[0] = wxRect(window_box.x - 1, top, window_box.width + 2, 1);
				// bottom
				strip_boxes[1] = wxRect(window_box.x - 1, bottom - 1, window_box.width + 2, 1);
				// left
				strip_boxes[2] = wxRect(window_box.x - 1, bottom, 1, height);
				// right
				strip_boxes[3] = wxRect(window_box.x + window_box.width, bottom, 1, height);

				for(int i = 0; i<4; i++)
				{
					MarkedObjectManyOfSame marked_object2;
					wxGetApp().m_marked_list->ObjectsInWindow(strip_boxes[i], &marked_object2, false);
					for(HeeksObj* object = marked_object2.GetFirstOfTopOnly(); object; object = marked_object2.Increment())if(object->GetType() != GripperType)
						obj_set.erase(object);
				}

				std::list<HeeksObj*> obj_list;
				for(std::set<HeeksObj*>::iterator It = obj_set.begin(); It != obj_set.end(); It++)
				{
					if(!event.ControlDown() || !wxGetApp().m_marked_list->ObjectMarked(*It))obj_list.push_back(*It);
				}
				wxGetApp().m_marked_list->Add(obj_list, true);
			}
			else{
				// select all the objects in the window, even if only partly in the window
				MarkedObjectManyOfSame marked_object;
				wxGetApp().m_marked_list->ObjectsInWindow(window_box, &marked_object, false);
				std::list<HeeksObj*> obj_list;
				for(HeeksObj* object = marked_object.GetFirstOfTopOnly(); object; object = marked_object.Increment())
				{
					if(object->GetType() != GripperType && (!event.ControlDown() || !wxGetApp().m_marked_list->ObjectMarked(object)))
						obj_list.push_back(object);
				}
				wxGetApp().m_marked_list->Add(obj_list, true);
			}
			wxGetApp().m_frame->m_graphics->DrawWindow(window_box, true); // undraw the window
			window_box_exists = false;
		}
		else
		{
			// select one object
			MarkedObjectOneOfEach marked_object;
			wxGetApp().FindMarkedObject(wxPoint(event.GetX(), event.GetY()), &marked_object);
			if(marked_object.m_map.size()>0){
				HeeksObj* previously_marked = NULL;
				if(wxGetApp().m_marked_list->size() == 1)
				{
					previously_marked = *(wxGetApp().m_marked_list->list().begin());
				}
				HeeksObj* o = marked_object.GetFirstOfTopOnly();
				HeeksObj* object = o;

				while(o)
				{
					if(o == previously_marked)
					{
						object = o;
						break;
					}

					o = marked_object.Increment();

					if(o)
					{
						// prefer highest order objects
						if(o->GetType() < object->GetType())object = o;
					}
				}
				if(!event.ShiftDown() && !event.ControlDown())
				{
					wxGetApp().m_marked_list->Clear(true);
				}
				if(wxGetApp().m_marked_list->ObjectMarked(object))
				{
					wxGetApp().m_marked_list->Remove(object, true);
				}
				else
				{
					wxGetApp().m_marked_list->Add(object, true);
					gp_Lin ray = wxGetApp().m_frame->m_graphics->m_view_point.SightLine(wxPoint(event.GetX(), event.GetY()));
					double ray_start[3], ray_direction[3];
					extract(ray.Location(), ray_start);
					extract(ray.Direction(), ray_direction);
					object->SetClickMarkPoint(&marked_object, ray_start, ray_direction);
				}
			}
			else
			{
				wxGetApp().m_marked_list->Clear(true);
			}
		}

		if(m_just_one && m_doing_a_main_loop && (wxGetApp().m_marked_list->size() > 0))
		{
			wxGetApp().ExitMainLoop();
		}
		else
		{
			wxGetApp().Repaint();
		}
	}
	else if(event.RightUp())
	{
		MarkedObjectOneOfEach marked_object;
		wxGetApp().FindMarkedObject(wxPoint(event.GetX(), event.GetY()), &marked_object);
		wxGetApp().DoDropDownMenu(wxGetApp().m_frame->m_graphics, wxPoint(event.GetX(), event.GetY()), &marked_object, false, true, event.ControlDown());
	}
	else if(event.Dragging())
	{
		if(event.MiddleIsDown())
		{
			wxPoint dm;
			dm.x = event.GetX() - CurrentPoint.x;
			dm.y = event.GetY() - CurrentPoint.y;
			if(wxGetApp().ctrl_does_rotate == event.ControlDown())
			{
				if(wxGetApp().m_rotate_mode)
				{
					wxGetApp().m_frame->m_graphics->m_view_point.Turn(dm);
				}
				else
				{
					wxGetApp().m_frame->m_graphics->m_view_point.TurnVertical(dm);
				}
			}
			else
			{
				wxGetApp().m_frame->m_graphics->m_view_point.Shift(dm, wxPoint(event.GetX(), event.GetY()));
			}
			wxGetApp().m_frame->m_graphics->Update();
			wxGetApp().m_frame->m_graphics->Refresh();
		}
		else if(event.LeftIsDown())
		{
			if(wxGetApp().drag_gripper)
			{
				double to[3], from[3];
				wxGetApp().m_digitizing->digitize(wxPoint(event.GetX(), event.GetY()));
				extract(wxGetApp().m_digitizing->digitized_point.m_point, to);
				wxGetApp().grip_to = wxGetApp().m_digitizing->digitized_point.m_point;
				extract(wxGetApp().grip_from, from);
				wxGetApp().drag_gripper->OnGripperMoved(from, to);
				wxGetApp().grip_from = make_point(from);
			}
			else if(abs(button_down_point.x - event.GetX())>2 || abs(button_down_point.y - event.GetY())>2)
			{
				if(!window_box_exists && m_can_grip_objects)
				{
					std::list<HeeksObj*> selected_objects_dragged;
					wxGetApp().m_show_grippers_on_drag = true;

					MarkedObjectManyOfSame marked_object;
					wxGetApp().FindMarkedObject(button_down_point, &marked_object);
					if(marked_object.m_map.size()>0){
						HeeksObj* object = marked_object.GetFirstOfTopOnly();
						double min_depth = 0.0;
						HeeksObj* closest_object = NULL;
						while(object)
						{
							if(wxGetApp().m_marked_list->ObjectMarked(object))
							{
								selected_objects_dragged = wxGetApp().m_marked_list->list();
								break;
							}
							double depth = marked_object.GetDepth();
							if(closest_object == NULL || depth<min_depth)
							{
								min_depth = depth;
								closest_object = object;
							}
							object = marked_object.Increment();
						}
						if(selected_objects_dragged.size() == 0 && closest_object){
							selected_objects_dragged.push_back(closest_object);
							wxGetApp().m_show_grippers_on_drag = false;
						}
					}

					if(selected_objects_dragged.size() > 0)
					{
						wxGetApp().drag_gripper = &drag_object_gripper;
						wxGetApp().m_digitizing->SetOnlyCoords(wxGetApp().drag_gripper, true);
						wxGetApp().m_digitizing->digitize(button_down_point);
						wxGetApp().grip_from = wxGetApp().m_digitizing->digitized_point.m_point;
						wxGetApp().grip_to = wxGetApp().grip_from;
						double from[3];
						from[0] = wxGetApp().grip_from.X();
						from[1] = wxGetApp().grip_from.Y();
						from[2] = wxGetApp().grip_from.Z();
						wxGetApp().drag_gripper->OnGripperGrabbed(selected_objects_dragged, wxGetApp().m_show_grippers_on_drag, from);
						wxGetApp().grip_from = gp_Pnt(from[0], from[1], from[2]);
						double to[3];
						wxGetApp().m_digitizing->digitize(wxPoint(event.GetX(), event.GetY()));
						extract(wxGetApp().m_digitizing->digitized_point.m_point, to);
						wxGetApp().grip_to = wxGetApp().m_digitizing->digitized_point.m_point;
						extract(wxGetApp().grip_from, from);
						wxGetApp().drag_gripper->OnGripperMoved(from, to);
						return;
					}
				}

				// do window selection
				if(!m_just_one)
				{
					wxGetApp().m_frame->m_graphics->SetXOR();
					if(window_box_exists)wxGetApp().m_frame->m_graphics->DrawWindow(window_box, true); // undraw the window
					window_box.x = button_down_point.x;
					window_box.width = event.GetX() - button_down_point.x;
					window_box.y = wxGetApp().m_frame->m_graphics->GetClientSize().GetHeight() - button_down_point.y;
					window_box.height = button_down_point.y - event.GetY();
					wxGetApp().m_frame->m_graphics->DrawWindow(window_box, true);// draw the window
					wxGetApp().m_frame->m_graphics->EndXOR();
					window_box_exists = true;
				}
			}
		}
		CurrentPoint = wxPoint(event.GetX(), event.GetY());
	}
	else if(event.Moving())
	{
#if 0
		MarkedObjectOneOfEach marked_object;
		wxGetApp().FindMarkedObject(wxPoint(event.GetX(), event.GetY()), &marked_object);
		wxGetApp().cursor_gripper = NULL;
		HeeksObj* object = marked_object.GetFirstOfTopOnly();
		while(object){
			if(object->GetType() == GripperType){
				wxGetApp().cursor_gripper = (Gripper*)object;
				break;
			}
			object = marked_object.Increment();
		}
		wxGetApp().m_frame->m_graphics->Refresh(0);
#endif
		CurrentPoint = wxPoint(event.GetX(), event.GetY());
	}

	if(event.GetWheelRotation() != 0)
	{
		double wheel_value = (double)(event.GetWheelRotation());
		double multiplier = wheel_value /1000.0;
		if(wxGetApp().mouse_wheel_forward_away)multiplier = -multiplier;
		double multiplier2 = 1 + multiplier;
		wxGetApp().m_frame->m_graphics->m_view_point.Scale(multiplier2);

		{
			// move towards mouse point, when zooming
			wxSize client_size = wxGetApp().m_frame->m_graphics->GetClientSize();
			double offset_x = (double)(event.GetX()) - (double)(client_size.GetWidth()) / 2;
			double offset_y = (double)(event.GetY()) - (double)(client_size.GetHeight()) / 2;

			offset_x *= multiplier / wxGetApp().GetPixelScale() * (multiplier > 0 ? 1.2 : -0.3);
			offset_y *= -multiplier / wxGetApp().GetPixelScale() * (multiplier > 0 ? 1.2 : -0.3);

			wxGetApp().m_frame->m_graphics->m_view_point.Shift(gp_Vec(offset_x, offset_y, 0));
		}

		wxGetApp().m_frame->m_graphics->Refresh();
	}

}

void CSelectMode::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case WXK_DELETE:
		wxGetApp().DeleteMarkedItems();
		return;

	case WXK_SHIFT:
		m_include_similar_objects = true;
		return;
	}

	CInputMode::OnKeyDown(event);
}

void CSelectMode::OnKeyUp(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case WXK_DELETE:
		return;

	case WXK_SHIFT:
		m_include_similar_objects = false;
		return;
	}

	CInputMode::OnKeyUp(event);
}

void CSelectMode::OnFrontRender(){
	if(wxGetApp().drag_gripper){
		wxGetApp().drag_gripper->OnFrontRender();
	}
	if(window_box_exists)wxGetApp().m_frame->m_graphics->DrawWindow(window_box, true);
}

bool CSelectMode::OnStart(){
	return true;
}

void CSelectMode::GetProperties(std::list<Property *> *list){
}

class EndPicking:public Tool{
public:
	void Run(){
		if(wxGetApp().m_select_mode->m_doing_a_main_loop)
		{
			wxGetApp().ExitMainLoop();
		}
		else{
			wxMessageBox(_T("Error! The \"Stop Picking\" button shouldn't have been available!"));
		}
	}
	const wxChar* GetTitle(){return _("Stop Picking");}
	wxString BitmapPath(){return _T("endpick");}
	const wxChar* GetToolTip(){return _("Finish picking");}
};

static EndPicking end_picking;

class PickAnything:public Tool{
public:
	void Run(){
		wxGetApp().m_marked_list->m_filter = -1;
	}
	const wxChar* GetTitle(){return _("Pick Anything");}
	wxString BitmapPath(){return _T("pickany");}
	const wxChar* GetToolTip(){return _("Set the selection filter to all items");}
};

static PickAnything pick_anything;

class PickEdges:public Tool{
public:
	void Run(){
		wxGetApp().m_marked_list->m_filter = MARKING_FILTER_EDGE;
	}
	const wxChar* GetTitle(){return _("Pick Edges");}
	wxString BitmapPath(){return _T("pickedges");}
	const wxChar* GetToolTip(){return _("Set the selection filter to only edges");}
};

static PickEdges pick_edges;

class PickFaces:public Tool{
public:
	void Run(){
		wxGetApp().m_marked_list->m_filter = MARKING_FILTER_FACE;
	}
	const wxChar* GetTitle(){return _("Pick Faces");}
	wxString BitmapPath(){return _T("pickfaces");}
	const wxChar* GetToolTip(){return _("Set the selection filter to only faces");}
};

static PickFaces pick_faces;

void CSelectMode::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	if(m_doing_a_main_loop)t_list->push_back(&end_picking);
	if(wxGetApp().m_marked_list->m_filter != -1)t_list->push_back(&pick_anything);
	if(wxGetApp().m_marked_list->m_filter != MARKING_FILTER_EDGE)t_list->push_back(&pick_edges);
	if(wxGetApp().m_marked_list->m_filter != MARKING_FILTER_FACE)t_list->push_back(&pick_faces);
}
