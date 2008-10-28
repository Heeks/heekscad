// ViewZooming.cpp
#include "stdafx.h"

#include "ViewZooming.h"
#include "SelectMode.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"

void ViewZooming::OnMouse( wxMouseEvent& event )
{
	if(event.LeftDown() || event.MiddleDown())
	{
		button_down_point = wxPoint(event.GetX(), event.GetY());
		CurrentPoint = button_down_point;
		wxGetApp().m_frame->m_graphics->StoreViewPoint();
		wxGetApp().m_frame->m_graphics->m_view_point.SetStartMousePoint(button_down_point);
	}
	else if(event.Dragging())
	{
		wxPoint dm;
		dm.x = event.GetX() - CurrentPoint.x;
		dm.y = event.GetY() - CurrentPoint.y;

		if(event.LeftIsDown())
		{
			wxGetApp().m_frame->m_graphics->m_view_point.Scale(wxPoint(event.GetX(), event.GetY()));
		}
		else if(event.MiddleIsDown())
		{
			wxGetApp().m_frame->m_graphics->m_view_point.Shift(dm, wxPoint(event.GetX(), event.GetY()));
		}

		wxGetApp().m_frame->m_graphics->Refresh(0);
		CurrentPoint = wxPoint(event.GetX(), event.GetY());
	}
	if(event.GetWheelRotation() != 0)wxGetApp().m_select_mode->OnMouse(event);
}
