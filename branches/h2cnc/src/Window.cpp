// Window.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Window.h"
#include "../interface/HeeksObj.h"
#include "SelectMode.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"

WindowDragging::WindowDragging(){
	reset();
}

void WindowDragging::reset(void){
	box_found = false;
	finish_dragging = false;
}

void WindowDragging::OnMouse( wxMouseEvent& event ){
	if(event.LeftDown()){
		window_box.x = event.GetX();
		window_box.y = wxGetApp().m_current_viewport->GetViewportSize().GetHeight() - event.GetY();
	}
	else if(event.LeftUp()){
		window_box.width = event.GetX() - window_box.x;
		window_box.height = (wxGetApp().m_current_viewport->GetViewportSize().GetHeight() - window_box.y) - event.GetY();
		if(abs(window_box.width)<4)box_found = false;
		else if(abs(window_box.height)<4)box_found = false;
		else box_found = true;
		finish_dragging = true;
	}
	else if(event.Dragging()){
		window_box.width = event.GetX() - window_box.x;
		window_box.height = (wxGetApp().m_current_viewport->GetViewportSize().GetHeight() - window_box.y) - event.GetY();
	}
}
