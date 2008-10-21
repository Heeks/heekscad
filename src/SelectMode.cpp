// SelectMode.cpp
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

bool CSelectMode::m_can_grip_objects = true;

CSelectMode::CSelectMode(){
	control_key_initially_pressed = false;
	window_box_exists = false;
	m_doing_a_main_loop = false;
	m_just_one = false;
}

const wxChar* CSelectMode::GetTitle()
{
	return m_doing_a_main_loop ? (m_prompt_when_doing_a_main_loop.c_str()):_T("Select Mode");
}

static GripperSelTransform drag_object_gripper(gp_Pnt(0, 0, 0), GripperTypeTranslate);

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
					wxGetApp().drag_gripper->OnGripperGrabbed(from);
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
			to[0] = wxGetApp().grip_to.X();
			to[1] = wxGetApp().grip_to.Y();
			to[2] = wxGetApp().grip_to.Z();
			from[0] = wxGetApp().grip_from.X();
			from[1] = wxGetApp().grip_from.Y();
			from[2] = wxGetApp().grip_from.Z();
			wxGetApp().drag_gripper->OnGripperReleased(from, to);
			wxGetApp().m_digitizing->SetOnlyCoords(wxGetApp().drag_gripper, false);
			wxGetApp().drag_gripper = NULL;
		}
		else if(window_box_exists)
		{
			wxGetApp().m_marked_list->Clear();
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
				for(std::set<HeeksObj*>::iterator It = obj_set.begin(); It != obj_set.end(); It++)obj_list.push_back(*It);
				wxGetApp().m_marked_list->Add(obj_list);
			}
			else{
				// select all the objects in the window, even if only partly in the window
				MarkedObjectManyOfSame marked_object;
				wxGetApp().m_marked_list->ObjectsInWindow(window_box, &marked_object, false);
				std::list<HeeksObj*> obj_list;
				for(HeeksObj* object = marked_object.GetFirstOfTopOnly(); object; object = marked_object.Increment())if(object->GetType() != GripperType)
					obj_list.push_back(object);
				wxGetApp().m_marked_list->Add(obj_list);
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
				HeeksObj* o = marked_object.GetFirstOfOneFromLevel();
				HeeksObj* object = o;
				while(o)
				{
					if(o == previously_marked)
					{
						object = o;
						break;
					}
					o = marked_object.Increment();
				}
				if(!event.ShiftDown() && !event.ControlDown())
				{
					wxGetApp().m_marked_list->Clear();
				}
				if(wxGetApp().m_marked_list->ObjectMarked(object))
				{
					wxGetApp().m_marked_list->Remove(object);
				}
				else
				{
					wxGetApp().m_marked_list->Add(object);
					gp_Lin ray = wxGetApp().m_frame->m_graphics->m_view_point.SightLine(wxPoint(event.GetX(), event.GetY()));
					double ray_start[3], ray_direction[3];
					extract(ray.Location(), ray_start);
					extract(ray.Direction(), ray_direction);
					object->SetClickMarkPoint(&marked_object, ray_start, ray_direction);
				}
			}
			else
			{
				wxGetApp().m_marked_list->Clear();
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
			wxGetApp().m_frame->m_graphics->Refresh(0);
		}
		else
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
				if(m_can_grip_objects)
				{
					MarkedObjectManyOfSame marked_object;
					wxGetApp().FindMarkedObject(button_down_point, &marked_object);
					bool selected_object_dragged = false;
					if(marked_object.m_map.size()>0)
					{
						HeeksObj* object = marked_object.GetFirstOfTopOnly();
						while(object)
						{
							if(wxGetApp().m_marked_list->ObjectMarked(object)){
								selected_object_dragged = true;
								break;
							}

							object = marked_object.Increment();
						}
					}

					if(!selected_object_dragged)
					{
						if(marked_object.m_map.size()>0)
						{
							HeeksObj* object = marked_object.GetFirstOfTopOnly();
							wxGetApp().m_marked_list->Clear();
							wxGetApp().m_marked_list->Add(object);
							wxGetApp().m_marked_list->create_grippers();
							selected_object_dragged = true;
						}
					}

					if(selected_object_dragged)
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
						wxGetApp().drag_gripper->OnGripperGrabbed(from);
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
		CurrentPoint = wxPoint(event.GetX(), event.GetY());
	}

	static double max_recorded_value = 0.0;

	if(event.GetWheelRotation() != 0)
	{
		double wheel_value = (double)(event.GetWheelRotation());
		double multiplier = wheel_value /1000.0;
		if(wxGetApp().mouse_wheel_forward_away)multiplier = -multiplier;
		wxGetApp().m_frame->m_graphics->m_view_point.Scale(multiplier);

		{
			// move towards mouse point, when zooming
			wxSize client_size = wxGetApp().m_frame->m_graphics->GetClientSize();
			double offset_x = (double)(event.GetX()) - (double)(client_size.GetWidth()) / 2;
			double offset_y = (double)(event.GetY()) - (double)(client_size.GetHeight()) / 2;

			offset_x *= multiplier / wxGetApp().GetPixelScale() * (multiplier > 0 ? 1.2 : -0.3);
			offset_y *= -multiplier / wxGetApp().GetPixelScale() * (multiplier > 0 ? 1.2 : -0.3);

			wxGetApp().m_frame->m_graphics->m_view_point.Shift(gp_Vec(offset_x, offset_y, 0));
		}

		wxGetApp().m_frame->m_graphics->Refresh(0);
	}

}

void CSelectMode::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case WXK_DELETE:
		wxGetApp().DeleteMarkedItems();
		return;
	}

	__super::OnKeyDown(event);
}

void CSelectMode::OnKeyUp(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case WXK_DELETE:
		return;
	}

	__super::OnKeyUp(event);
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

void CSelectMode::GetOptions(std::list<Property *> *list){
}

class EndPicking:public Tool{
private:
	static wxBitmap* m_bitmap;

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
	const wxChar* GetTitle(){return _T("Stop Picking");}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/endpick.png"), wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const wxChar* GetToolTip(){return _T("Finish picking");}
};
wxBitmap* EndPicking::m_bitmap = NULL;

static EndPicking end_picking;


void CSelectMode::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	if(m_doing_a_main_loop)t_list->push_back(&end_picking);
}
