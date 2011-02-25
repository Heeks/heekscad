// ViewRotating.cp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ViewRotating.h"
#include "SelectMode.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"

void ViewRotating::OnMouse( wxMouseEvent& event )
{
	if(event.LeftDown() || event.MiddleDown())
	{
		button_down_point = wxPoint(event.GetX(), event.GetY());
		CurrentPoint = button_down_point;
		wxGetApp().m_current_viewport->StoreViewPoint();
		wxGetApp().m_current_viewport->m_view_point.SetStartMousePoint(button_down_point);
	}
	else if(event.Dragging())
	{
		wxPoint dm;
		dm.x = event.GetX() - CurrentPoint.x;
		dm.y = event.GetY() - CurrentPoint.y;

		if(event.LeftIsDown())
		{
			if(wxGetApp().m_rotate_mode)
			{
				wxGetApp().m_current_viewport->m_view_point.Turn(dm);
			}
			else
			{
				wxGetApp().m_current_viewport->m_view_point.TurnVertical(dm);
			}
		}
		else if(event.MiddleIsDown())
		{
			wxGetApp().m_current_viewport->m_view_point.Shift(dm, wxPoint(event.GetX(), event.GetY()));
		}

		wxGetApp().m_frame->m_graphics->Refresh();
		CurrentPoint = wxPoint(event.GetX(), event.GetY());
	}
	else if(event.RightUp()){
		// do context menu same as select mode
		wxGetApp().m_select_mode->OnMouse(event);
	}
	if(event.GetWheelRotation() != 0)wxGetApp().m_select_mode->OnMouse(event);

}

class EndRotating:public Tool{
public:
	CInputMode* m_saved_mode;

	void Run(){
		wxGetApp().input_mode_object = m_saved_mode;
	}
	const wxChar* GetTitle(){return _("Stop rotating");}
	wxString BitmapPath(){return _T("endpick");}
};

static EndRotating end_rotating;

bool ViewRotating::OnModeChange(void)
{
	end_rotating.m_saved_mode = wxGetApp().input_mode_object;
	return true;
}

void ViewRotating::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	t_list->push_back(&end_rotating);
}
