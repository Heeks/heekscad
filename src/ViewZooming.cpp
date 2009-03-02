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
			wxGetApp().m_frame->m_graphics->m_view_point.Scale(wxPoint(event.GetX(), event.GetY()), m_reversed);
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

static wxString str_for_GetHelpText;

const wxChar* ViewZooming::GetHelpText(){
	str_for_GetHelpText = wxString(_("Drag with the left mouse button")) + _T("\n") + (m_reversed ? _("Forward to zoom in, Back to zoom out"):_("Back to zoom in, Forward to zoom out")) + _T("\n") + _("Hold middle mouse button down to pan");
	return str_for_GetHelpText;
}
