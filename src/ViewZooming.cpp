// ViewZooming.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ViewZooming.h"
#include "SelectMode.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"

bool ViewZooming::m_reversed = false;

void ViewZooming::OnMouse( wxMouseEvent& event )
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
			wxGetApp().m_current_viewport->m_view_point.Scale(wxPoint(event.GetX(), event.GetY()), m_reversed);
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

static wxString str_for_GetHelpText;

const wxChar* ViewZooming::GetHelpText(){
	str_for_GetHelpText = wxString(_("Drag with the left mouse button")) + _T("\n") + (m_reversed ? _("Forward to zoom in, Back to zoom out"):_("Back to zoom in, Forward to zoom out")) + _T("\n") + _("Hold middle mouse button down to pan");
	return str_for_GetHelpText;
}

class EndZooming:public Tool{
public:
	CInputMode* m_saved_mode;

	void Run(){
		wxGetApp().input_mode_object = m_saved_mode;
	}
	const wxChar* GetTitle(){return _("Stop zooming");}
	wxString BitmapPath(){return _T("endpick");}
};

static EndZooming end_zooming;

bool ViewZooming::OnModeChange(void)
{
	end_zooming.m_saved_mode = wxGetApp().input_mode_object;
	return true;
}

void ViewZooming::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	t_list->push_back(&end_zooming);
}
