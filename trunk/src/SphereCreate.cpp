// SphereCreate.cpp

#include "stdafx.h"
#include "SphereCreate.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyString.h"
#include "../interface/PropertyChoice.h"
#include "../interface/Tool.h"
#include "../interface/Material.h"
#include "PropertyVertex.h"
#include "DigitizeMode.h"
#include "Solid.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"
#include "SelectMode.h"

class CSphereApply:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		// add a sphere with using m_pos, m_r
		TopoDS_Solid solid = BRepPrimAPI_MakeSphere(gp_Pnt(sphere_creator.m_pos[0], sphere_creator.m_pos[1], sphere_creator.m_pos[2]), sphere_creator.m_r);
		wxGetApp().AddUndoably(new CSolid(solid, "Sphere"), NULL, NULL);
		wxGetApp().SetInputMode(wxGetApp().m_select_mode);
		wxGetApp().Repaint();
	}
	const char* GetTitle(){return "Apply";}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + "/bitmaps/apply.png", wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const char* GetToolTip(){return "Create Sphere, and finish";}
};
wxBitmap* CSphereApply::m_bitmap = NULL;

CSphereApply sphere_apply;

CSphereCreate sphere_creator;

CSphereCreate::CSphereCreate()
{
	m_r = 5.0;
	m_pos[0] = 0.0;
	m_pos[1] = 0.0;
	m_pos[2] = 0.0;
	m_mode = 0;
}

void CSphereCreate::SetPositionOrRadius(const wxPoint& point)
{
		DigitizeType type_found = wxGetApp().m_digitizing->digitize(point);
		if(type_found != DigitizeNoItemType)
		{
			gp_Pnt p = wxGetApp().m_digitizing->position_found;
			switch(m_mode)
			{
			case 0:
				{
					// position
					extract(p, m_pos);
				}
				break;

			case 1:
				{
					// radius
					gp_Vec v(make_point(m_pos), p);
					m_r = v.Magnitude();
				}
				break;
			}
			wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
			wxGetApp().Repaint(true);
		}
}

void CSphereCreate::OnMouse( wxMouseEvent& event )
{
	bool event_used = false;
	if(LeftAndRightPressed(event, event_used))
	{
		sphere_apply.Run();
	}

	if(event_used)return;

	if(event.MiddleIsDown() || event.GetWheelRotation() != 0)
	{
		wxGetApp().m_select_mode->OnMouse(event);
	}
	else{
		if(event.Dragging())
		{
			SetPositionOrRadius(event.GetPosition());
		}

		if(event.LeftDown()){
			SetPositionOrRadius(event.GetPosition());
		}
	}
}

void CSphereCreate::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case 'R':
		// switch to radius mode, until released
		m_mode = 1;
		wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
		wxGetApp().Repaint();
		return;
	}

	__super::OnKeyDown(event);
}

void CSphereCreate::OnKeyUp(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case 'R':
		// switch back to position mode
		m_mode = 0;
		wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
		wxGetApp().Repaint();
		return;
	}

	__super::OnKeyUp(event);
}

bool CSphereCreate::OnStart()
{
	m_r = 5.0;
	m_pos[0] = 0.0;
	m_pos[1] = 0.0;
	m_pos[2] = 0.0;
	m_mode = false;

	return true;
}

void CSphereCreate::OnRender()
{
	// draw a sphere
	glEnable(GL_LIGHTING);
	Material().glMaterial(0.5);
	wxGetApp().glSphere(m_r, m_pos);
	glDisable(GL_BLEND);
	glDepthMask(1);
	glDisable(GL_LIGHTING);
}

static void set_x(double value){sphere_creator.m_pos[0] = value; wxGetApp().Repaint();}
static void set_y(double value){sphere_creator.m_pos[1] = value; wxGetApp().Repaint();}
static void set_z(double value){sphere_creator.m_pos[2] = value; wxGetApp().Repaint();}
static void set_r(double value){sphere_creator.m_r = value; wxGetApp().Repaint();}
static void set_mode(int value){sphere_creator.m_mode = value; wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();}

void CSphereCreate::GetProperties(std::list<Property *> *list)
{
	std::list< std::string > choices;
	choices.push_back(std::string("choose position"));
	choices.push_back(std::string("choose radius"));
	list->push_back(new PropertyChoice("drag mode", choices, m_mode, set_mode));
	list->push_back(new PropertyString("(press 'r' for radius)", ""));

	// add x, y, z and radius for the "create" button
	list->push_back(new PropertyDouble("X", m_pos[0], set_x));
	list->push_back(new PropertyDouble("Y", m_pos[1], set_y));
	list->push_back(new PropertyDouble("Z", m_pos[2], set_z));
	list->push_back(new PropertyDouble("radius", m_r, set_r));
}

class CSphereCancel:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		// return to Select mode
		wxGetApp().SetInputMode(wxGetApp().m_select_mode);
		wxGetApp().Repaint();
	}
	const char* GetTitle(){return "Cancel";}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + "/bitmaps/cancel.png", wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const char* GetToolTip(){return "Finish without creating sphere";}
};
wxBitmap* CSphereCancel::m_bitmap = NULL;

CSphereCancel sphere_cancel;

void CSphereCreate::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	// add a do it now button
	t_list->push_back(&sphere_apply);
	t_list->push_back(&sphere_cancel);
}
