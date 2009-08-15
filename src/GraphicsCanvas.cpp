// GraphicsCanvas.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "GraphicsCanvas.h"
#include "SelectMode.h"
#include "../interface/MarkedObject.h"
#include "MarkedList.h"
#include "../interface/Material.h"
#include "HeeksFrame.h"
#include "../interface/HeeksCADInterface.h"

extern CHeeksCADInterface heekscad_interface;

#define wxID_TEST1 10001
#define wxID_TEST2 10002

BEGIN_EVENT_TABLE(CGraphicsCanvas, wxGLCanvas)
    EVT_SIZE(CGraphicsCanvas::OnSize)
	EVT_ERASE_BACKGROUND(CGraphicsCanvas::OnEraseBackground)
    EVT_PAINT(CGraphicsCanvas::OnPaint)
    EVT_MOUSE_EVENTS(CGraphicsCanvas::OnMouse)
    EVT_MENU_RANGE(ID_FIRST_POP_UP_MENU_TOOL, ID_FIRST_POP_UP_MENU_TOOL + 1000, CGraphicsCanvas::OnMenuEvent)
	EVT_KEY_DOWN(CGraphicsCanvas::OnKeyDown)
	EVT_KEY_UP(CGraphicsCanvas::OnKeyUp)
	EVT_CHAR(CGraphicsCanvas::OnCharEvent)
END_EVENT_TABLE()

CGraphicsCanvas::CGraphicsCanvas(wxWindow* parent, int *attribList)
        : wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _T("some text"), attribList),m_frozen(false), m_refresh_wanted_on_thaw(false)
{
	m_render_on_front_done = false;

	wxGetApp().RegisterObserver(this);
}

void CGraphicsCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    /* must always be here */
    wxPaintDC dc(this);

#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif

    SetCurrent();

	glDrawBuffer(GL_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if(wxGetApp().m_antialiasing)
	{
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	}
	else
	{
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}

	m_view_point.SetViewport();
	m_view_point.SetProjection(true);
	m_view_point.SetModelview();


	wxGetApp().background_color.glClearColor(wxGetApp().m_antialiasing ? 0.0f : 1.0f);

	// clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render everything
	wxGetApp().glCommandsAll(false, m_view_point);

	// mark various XOR drawn items as not drawn
	m_render_on_front_done = false;

    SwapBuffers();
}

void CGraphicsCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);
	Refresh();
}

void CGraphicsCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
	// Do nothing, to avoid flashing on MSW
}

void CGraphicsCanvas::DrawFront(void){
	if(!m_render_on_front_done){
		FrontRender();
		m_render_on_front_done = true;
	}
}

void CGraphicsCanvas::EndDrawFront(void){
	if(m_render_on_front_done){
		FrontRender();
		m_render_on_front_done = false;
	}
}

void CGraphicsCanvas::FrontRender(void){
	m_view_point.SetViewport();
	m_view_point.SetProjection(false);
	m_view_point.SetModelview();
	
	SetXOR();
	
	wxGetApp().input_mode_object->OnFrontRender();
	
	EndXOR();
	
	m_render_on_front_done = true;
}

void CGraphicsCanvas::SetIdentityProjection(void){
    int w, h;
    GetClientSize(&w, &h);
	glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(- 0.5, w - 0.5, -0.5, h - 0.5, 0,10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void CGraphicsCanvas::SetXOR(void){
	glGetIntegerv(GL_DRAW_BUFFER, &m_save_buffer_for_XOR);
	glDrawBuffer(GL_FRONT);
	glDepthMask(0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_XOR);
	glColor3ub(255, 255, 255);
}

void CGraphicsCanvas::EndXOR(void){
	glDisable(GL_COLOR_LOGIC_OP);
	glDepthMask(1);
	glEnable(GL_DEPTH_TEST);
	glDrawBuffer(m_save_buffer_for_XOR);
	glFlush();
}

void CGraphicsCanvas::OnMouse( wxMouseEvent& event )
{
	if(event.Entering()){
	    SetCurrent();
		SetFocus(); // so middle wheel works
	}

	wxGetApp().input_mode_object->OnMouse( event );

	for(std::list< void(*)(wxMouseEvent&) >::iterator It = wxGetApp().m_lbutton_up_callbacks.begin(); It != wxGetApp().m_lbutton_up_callbacks.end(); It++)
	{
		void(*callbackfunc)(wxMouseEvent& event) = *It;
		(*callbackfunc)(event);
	}

	event.Skip();
}

void CGraphicsCanvas::OnKeyDown(wxKeyEvent& event)
{
	if(event.GetKeyCode() == WXK_ESCAPE && wxGetApp().m_frame->IsFullScreen())wxGetApp().m_frame->ShowFullScreen(false);
	else
	{
		if(event.GetKeyCode() == WXK_ESCAPE && wxGetApp().EndSketchMode())
		{}
			else wxGetApp().input_mode_object->OnKeyDown(event);
	}
	event.Skip();
}

void CGraphicsCanvas::OnCharEvent(wxKeyEvent& event)
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
				for(HeeksObj* object = heekscad_interface.GetFirstObject(); object != NULL; object = heekscad_interface.GetNextObject())
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
			wxGetApp().Paste(NULL);
			wxGetApp().Repaint();
			event.Skip();
			break;

		default:
			break;
	} // End switch
} // End OnCharEvent() method


void CGraphicsCanvas::OnKeyUp(wxKeyEvent& event)
{
	wxGetApp().input_mode_object->OnKeyUp(event);
	event.Skip();
}

void CGraphicsCanvas::OnMenuEvent(wxCommandEvent& event)
{
	wxGetApp().on_menu_event(event);
}

void CGraphicsCanvas::OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)
{
	Refresh();
}

void CGraphicsCanvas::WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj *>* added_list, const std::list<HeeksObj *>* removed_list)
{
	Refresh();
}

void CGraphicsCanvas::Clear()
{
	Refresh();
}

void CGraphicsCanvas::Freeze()
{
	m_frozen = true;
}

void CGraphicsCanvas::Thaw()
{
	m_frozen = false;
	if(m_refresh_wanted_on_thaw)
	{
		Refresh();
		m_refresh_wanted_on_thaw = false;
	}
}

void CGraphicsCanvas::Refresh()
{
	if(m_frozen)
	{
		m_refresh_wanted_on_thaw = true;
	}
	else
	{
		wxGLCanvas::Refresh(false);
	}
}

void CGraphicsCanvas::RefreshSoon()
{
	if(m_frozen)
	{
		m_refresh_wanted_on_thaw = true;
	}
	else if(wxGetApp().m_frame->IsShown())
	{
		wxGLCanvas::Refresh(false);
		Update();
	}
}

void CGraphicsCanvas::OnMagExtents(bool rotate, bool recalculate_gl_lists) 
{
	m_view_points.clear();
	if(rotate){
		m_orthogonal = true;
		SetViewPoint();
	}
	else{
		m_view_point.SetViewAroundAllObjects();
		StoreViewPoint();
	}

	if(recalculate_gl_lists)
		wxGetApp().RecalculateGLLists();

	Refresh();
}

void CGraphicsCanvas::OnMagPrevious()
{
	RestorePreviousViewPoint();
	Refresh();
}

void CGraphicsCanvas::SetViewPoint(void){
	if(m_orthogonal){
		gp_Vec vx, vy, vz;
		m_view_point.GetTwoAxes(vx, vy, false, 0);
		{
			gp_Vec right = m_view_point.rightwards_vector().Normalized();
			gp_Vec vertical = m_view_point.m_vertical.Normalized();
			gp_Vec v_choices[4] = {vx, vy, -vx, -vy};
			gp_Vec *best_x = NULL, *best_y = NULL;
			double best_x_dp = 0, best_y_dp = 0;
			for(int i = 0; i<4; i++){
				double x_dp = v_choices[i] * right;
				double y_dp = v_choices[i] * vertical;
				if(i == 0 || x_dp > best_x_dp){
					best_x_dp = x_dp;
					best_x = &v_choices[i];
				}
				if(i == 0 || y_dp > best_y_dp){
					best_y_dp = y_dp;
					best_y = &v_choices[i];
				}
			}

			vy = *best_y;				
			vz = vx ^ vy;

			m_view_point.SetView(vy, vz);
			StoreViewPoint();
			return;
		}
	}

	gp_Vec vy(0, 1, 0), vz(0, 0, 1);
	m_view_point.SetView(vy, vz);
	StoreViewPoint();
}

void CGraphicsCanvas::StoreViewPoint(void){
	m_view_points.push_back(m_view_point);
}

void CGraphicsCanvas::RestorePreviousViewPoint(void){
	if(m_view_points.size()>0){
		m_view_point = m_view_points.back();
		m_view_points.pop_back();
	}
}

void CGraphicsCanvas::DrawObjectsOnFront(const std::list<HeeksObj*> &list, bool do_depth_testing){
	m_view_point.SetViewport();
	m_view_point.SetProjection(do_depth_testing);
	m_view_point.SetModelview();
	
	glDrawBuffer(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	wxGetApp().CreateLights();
	glDisable(GL_LIGHTING);
	Material().glMaterial(1.0);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);

	wxGetApp().m_frame->m_graphics->m_view_point.SetPolygonOffset();

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		object->glCommands(false, false, false);
	}

	wxGetApp().DestroyLights();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL );
	glFlush();
}


bool CGraphicsCanvas::UsePreviousViewPoint(void){
	if(m_view_points.size() == 0){
		return false;
	}
	else{
		RestorePreviousViewPoint();
		Refresh();
		return true;
	}
}

void CGraphicsCanvas::WindowMag(wxRect &window_box){
	StoreViewPoint();
	m_view_point.WindowMag(window_box);
	Refresh();
}

void CGraphicsCanvas::FindMarkedObject(const wxPoint &point, MarkedObject* marked_object){
	wxGetApp().m_marked_list->FindMarkedObject(point, marked_object);
}

void CGraphicsCanvas::DrawWindow(wxRect &rect, bool allow_extra_bits){
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    int w, h;
    GetClientSize(&w, &h);
	glOrtho(- 0.5, w - 0.5, -0.5, h - 0.5, 0,10);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	int x1 = rect.x;
	int y1 = rect.y;
	int x2 = rect.x + rect.width;
	int y2 = rect.y + rect.height;

	glBegin(GL_LINE_STRIP);
	glVertex2f((GLfloat)x1, (GLfloat)y2);
	glVertex2f((GLfloat)x1, (GLfloat)y1);
	glVertex2f((GLfloat)x2, (GLfloat)y1);
	glVertex2f((GLfloat)x2, (GLfloat)y2);
	glVertex2f((GLfloat)x1, (GLfloat)y2);
	glEnd();

	// draw extra bits
	if(rect.width < 0 && allow_extra_bits){
		int extra_x = -5;
		int extra_y = -5;
		if(rect.height > 0)extra_y = -extra_y;

		glBegin(GL_LINE_STRIP);
		glVertex2f((GLfloat)x1 - extra_x, (GLfloat)y1);
		glVertex2f((GLfloat)x1 - extra_x, (GLfloat)y1 - extra_y);
		glVertex2f((GLfloat)x1, (GLfloat)y1 - extra_y);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2f((GLfloat)x2 + extra_x, (GLfloat)y1);
		glVertex2f((GLfloat)x2 + extra_x, (GLfloat)y1 - extra_y);
		glVertex2f((GLfloat)x2, (GLfloat)y1 - extra_y);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2f((GLfloat)x1 - extra_x, (GLfloat)y2);
		glVertex2f((GLfloat)x1 - extra_x, (GLfloat)y2 + extra_y);
		glVertex2f((GLfloat)x1, (GLfloat)y2 + extra_y);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2f((GLfloat)x2 + extra_x, (GLfloat)y2);
		glVertex2f((GLfloat)x2 + extra_x, (GLfloat)y2 + extra_y);
		glVertex2f((GLfloat)x2, (GLfloat)y2 + extra_y);
		glEnd();
	}
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
    glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}
