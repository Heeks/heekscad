// MagDragWindow.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "MagDragWindow.h"
#include "SelectMode.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"

void MagDragWindow::OnMouse( wxMouseEvent& event )
{
	if(event.LeftDown())
	{
		window_box.x = event.GetX();
		window_box.y = wxGetApp().m_current_viewport->GetViewportSize().GetHeight() - event.GetY();
		window_box_exists = false;
	}
	else if(event.LeftUp())
	{
		window_box.width = event.GetX() - window_box.x;
		window_box.height = (wxGetApp().m_current_viewport->GetViewportSize().GetHeight() - event.GetY()) - window_box.y;
		if(abs(window_box.width)<4){wxGetApp().SetInputMode(save_input_mode); return;}
		if(abs(window_box.height)<4){wxGetApp().SetInputMode(save_input_mode); return;}

		wxGetApp().m_frame->m_graphics->WindowMag(window_box);
		window_box_exists = false;
		wxGetApp().SetInputMode(save_input_mode);
	}
	else if(event.Dragging())
	{
		wxGetApp().m_frame->m_graphics->SetCurrent();
		wxGetApp().m_current_viewport->SetXOR();
		if(window_box_exists)wxGetApp().m_current_viewport->DrawWindow(window_box, false);// undraw the window
		window_box.width = event.GetX() - window_box.x;
		window_box.height = (wxGetApp().m_current_viewport->GetViewportSize().GetHeight() - event.GetY()) - window_box.y;
		wxGetApp().m_current_viewport->DrawWindow(window_box, false);// draw the window
		wxGetApp().m_current_viewport->EndXOR();
		window_box_exists = true;
	}
}

bool MagDragWindow::OnModeChange(void){
	save_input_mode = wxGetApp().input_mode_object;
	return true;
}

void MagDragWindow::OnFrontRender(){
	if(window_box_exists){
		wxGetApp().m_current_viewport->DrawWindow(window_box, false);
	}
}
