// DigitizeMode.cpp
#include "stdafx.h"
#include "DigitizeMode.h"
#include "../interface/MarkedObject.h"
#include "../interface/PropertyList.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyString.h"
#include "../interface/Tool.h"
#include "SelectMode.h"
#include "MarkedList.h"
#include "PointOrWindow.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"
#include "OptionsCanvas.h"
#include "CoordinateSystem.h"

DigitizeMode::DigitizeMode(){
	point_or_window = new PointOrWindow(false);
	m_doing_a_main_loop = false;
	m_callback = NULL;
}

DigitizeMode::~DigitizeMode(void){
	delete point_or_window;
}

const wxChar* DigitizeMode::GetTitle()
{
	return m_doing_a_main_loop ? (m_prompt_when_doing_a_main_loop.c_str()):_("Digitize Mode");
}

void DigitizeMode::OnMouse( wxMouseEvent& event ){
	if(event.MiddleIsDown() || event.GetWheelRotation() != 0)
	{
		wxGetApp().m_select_mode->OnMouse(event);
		return;
	}

	if(event.LeftDown()){
		point_or_window->OnMouse(event);
		lbutton_point = digitize(wxPoint(event.GetX(), event.GetY()));
	}
	else if(event.LeftUp()){
		if(lbutton_point.m_type != DigitizeNoItemType){
			digitized_point = lbutton_point;
			if(m_doing_a_main_loop){
				wxGetApp().ExitMainLoop();
			}
		}
	}
	else if(event.Moving()){
		digitize(wxPoint(event.GetX(), event.GetY()));
		point_or_window->OnMouse(event);
		if(m_doing_a_main_loop)wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
		if(m_callback)
		{
			double pos[3];
			extract(digitized_point.m_point, pos);
			(*m_callback)(pos);
		}
	}
}

static gp_Trsf global_matrix_relative_to_screen;

static const gp_Trsf& digitizing_matrix(bool calculate = false){
	if(calculate){
		if(wxGetApp().digitize_screen){
			gp_Trsf mat = wxGetApp().GetDrawMatrix(false);
			gp_Pnt origin = gp_Pnt(0, 0, 0).Transformed(mat);
			gp_Pnt x1 = origin.XYZ() + gp_XYZ(1, 0, 0);
			gp_Pnt y1 = origin.XYZ() + gp_XYZ(0, 1, 0);
			gp_Pnt po = origin;
			po = wxGetApp().m_frame->m_graphics->m_view_point.glUnproject(po);
			x1 = wxGetApp().m_frame->m_graphics->m_view_point.glUnproject(x1);
			y1 = wxGetApp().m_frame->m_graphics->m_view_point.glUnproject(y1);
			
			global_matrix_relative_to_screen = make_matrix(origin, gp_Vec(po, x1).Normalized(), gp_Vec(po, y1).Normalized());
		}
		else{
			wxGetApp().m_frame->m_graphics->m_view_point.Set90PlaneDrawMatrix(global_matrix_relative_to_screen);
		}
	}
	return global_matrix_relative_to_screen;
}

bool DigitizeMode::OnModeChange(void){
	point_or_window->reset();
	if(!point_or_window->OnModeChange())return false;
	digitize(wxGetApp().cur_mouse_pos);
	return true;
}

DigitizedPoint DigitizeMode::digitize1(const wxPoint &input_point){
	gp_Lin ray = wxGetApp().m_frame->m_graphics->m_view_point.SightLine(input_point);
	std::list<DigitizedPoint> compare_list;
	MarkedObjectManyOfSame marked_object;
	if(wxGetApp().digitize_end || wxGetApp().digitize_inters || wxGetApp().digitize_centre || wxGetApp().digitize_midpoint || wxGetApp().digitize_nearest || wxGetApp().digitize_tangent){
		point_or_window->SetWithPoint(input_point);
		wxGetApp().m_marked_list->ignore_coords_only = true;
		wxGetApp().m_marked_list->ObjectsInWindow(point_or_window->box_chosen, &marked_object);
		wxGetApp().m_marked_list->ignore_coords_only = false;
	}
	if(wxGetApp().digitize_end){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				std::list<double> vl;
				object->GetGripperPositions(&vl, true);
				std::list<gp_Pnt> plist;
				convert_doubles_to_pnts(vl, plist, true);
				for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
				{
					gp_Pnt& pnt = *It;
					compare_list.push_back(DigitizedPoint(pnt, DigitizeEndofType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(wxGetApp().digitize_inters){
		if(marked_object.m_map.size()>0){
			std::list<HeeksObj*> object_list;
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				object_list.push_back(object);
				object = marked_object.Increment();
			}

			if(object_list.size() > 1)
			{
				for(std::list<HeeksObj*>::iterator It = object_list.begin(); It != object_list.end(); It++)
				{
					HeeksObj* object = *It;
					std::list<HeeksObj*>::iterator It2 = It;
					It2++;
					for(; It2 != object_list.end(); It2++)
					{
						HeeksObj* object2 = *It2;
						std::list<double> rl;
						if(object->Intersects(object2, &rl))
						{
							std::list<gp_Pnt> plist;
							convert_doubles_to_pnts(rl, plist);
							for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
							{
								gp_Pnt& pnt = *It;
								compare_list.push_back(DigitizedPoint(pnt, DigitizeIntersType));
							}
						}
					}
				}
			}
		}
	}
	if(wxGetApp().digitize_midpoint){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				double p[3];
				if(object->GetMidPoint(p)){
					compare_list.push_back(DigitizedPoint(make_point(p), DigitizeMidpointType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(wxGetApp().digitize_nearest){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfEverything();
			while(object){
				double ray_start[3], ray_direction[3];
				extract(ray.Location(), ray_start);
				extract(ray.Direction(), ray_direction);
				double p[3];
				if(object->FindNearPoint(ray_start, ray_direction, p)){
					compare_list.push_back(DigitizedPoint(make_point(p), DigitizeNearestType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(wxGetApp().digitize_tangent){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfEverything();
			while(object){
				double ray_start[3], ray_direction[3];
				extract(ray.Location(), ray_start);
				extract(ray.Direction(), ray_direction);
				double p[3];
				if(object->FindPossTangentPoint(ray_start, ray_direction, p)){
					compare_list.push_back(DigitizedPoint(make_point(p), DigitizeTangentType, object));
				}
				object = marked_object.Increment();
			}
		}
	}
	double min_dist = -1;
	double best_dp = 0;
	DigitizedPoint *best_digitized_point = NULL;
	if(compare_list.size() >0){
		std::list<DigitizedPoint>::iterator It;
		double dist;
		double dp;
		for(It = compare_list.begin(); It != compare_list.end(); It++){
			DigitizedPoint *this_digitized_point = &(*It);
			dist = ray.Distance(this_digitized_point->m_point);
			dp = gp_Vec(ray.Direction()) * gp_Vec(this_digitized_point->m_point.XYZ()) - gp_Vec(ray.Direction()) * gp_Vec(ray.Location().XYZ());
			if(dist * wxGetApp().GetPixelScale() < 2)dist = 2/wxGetApp().GetPixelScale();
			if(dist * wxGetApp().GetPixelScale()>10)continue;
			bool use_this = false;
			if(best_digitized_point == NULL)use_this = true;
			else if(this_digitized_point->importance() > best_digitized_point->importance())use_this = true;
			else if(this_digitized_point->importance() == best_digitized_point->importance() && dist<min_dist)use_this = true;
			if(use_this){
				min_dist = dist;
				best_digitized_point = this_digitized_point;
				best_dp = dp;
			}
		}
	}
	if(wxGetApp().digitize_centre && (min_dist == -1 || min_dist * wxGetApp().GetPixelScale()>5)){
		gp_Pnt pos;
		for(HeeksObj* object = marked_object.GetFirstOfEverything(); object != NULL; object = marked_object.Increment()){
			double p[3];
			if(object->GetCentrePoint(p)){
				compare_list.push_back(DigitizedPoint(make_point(p), DigitizeCentreType));
				best_digitized_point = &(compare_list.back());
				break;
			}
		}
	}
	DigitizedPoint point;
	if(best_digitized_point){
		point = *best_digitized_point;
	}
	else if(wxGetApp().digitize_coords){
		point = Digitize(ray);
	}
	
	return point;
}

DigitizedPoint DigitizeMode::Digitize(const gp_Lin &ray){
	gp_Pln pl(gp_Pnt(0, 0, 0), gp_Vec(0, 0, 1));
	pl.Transform(digitizing_matrix(true));
	gp_Pnt pnt;
	if(!intersect(ray, pl, pnt)){
		pl = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, -1, 0));
		if(!intersect(ray, pl, pnt))DigitizedPoint();

		pl = gp_Pln(gp_Pnt(0, 0, 0), gp_Vec(1, 0, 0));
		if(!intersect(ray, pl, pnt))DigitizedPoint();
	}

	DigitizedPoint point(pnt, DigitizeCoordsType);

	if(wxGetApp().draw_to_grid){
		gp_Vec plane_vx = gp_Vec(1, 0, 0).Transformed(digitizing_matrix());
		gp_Vec plane_vy = gp_Vec(0, 1, 0).Transformed(digitizing_matrix());
		gp_Pnt datum = gp_Pnt(0, 0, 0).Transformed(digitizing_matrix());

		double a = gp_Vec(datum.XYZ()) * plane_vx;
		double b = gp_Vec(point.m_point.XYZ()) * plane_vx;
		double c = b - a;
		double extra1 = c > -0.00000001 ? 0.5:-0.5;
		c = (int)(c / wxGetApp().digitizing_grid + extra1) * wxGetApp().digitizing_grid;

		double datum_dotp_y = gp_Vec(datum.XYZ()) * plane_vy;
		double rp_dotp_y = gp_Vec(point.m_point.XYZ()) * plane_vy;
		double d = rp_dotp_y - datum_dotp_y;
		double extra2 = d > -0.00000001 ? 0.5:-0.5;
		d = (int)(d / wxGetApp().digitizing_grid + extra2) * wxGetApp().digitizing_grid;

		point.m_point = datum.XYZ() + plane_vx.XYZ() * c + plane_vy.XYZ() * d;
	}

	return point;
}

DigitizedPoint DigitizeMode::digitize(const wxPoint &point){
	digitized_point = digitize1(point);
	return digitized_point;
}

void DigitizeMode::OnFrontRender(){
	point_or_window->OnFrontRender();
}

static void set_x(double value, HeeksObj* object){wxGetApp().m_digitizing->digitized_point.m_point.SetX(value); wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();}
static void set_y(double value, HeeksObj* object){wxGetApp().m_digitizing->digitized_point.m_point.SetY(value); wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();}
static void set_z(double value, HeeksObj* object){wxGetApp().m_digitizing->digitized_point.m_point.SetZ(value); wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();}

void DigitizeMode::GetProperties(std::list<Property *> *list){
	list->push_back(new PropertyDouble(_("X"), digitized_point.m_point.X(), NULL, set_x));
	list->push_back(new PropertyDouble(_("Y"), digitized_point.m_point.Y(), NULL, set_y));
	list->push_back(new PropertyDouble(_("Z"), digitized_point.m_point.Z(), NULL, set_z));
}

void DigitizeMode::GetOptions(std::list<Property *> *list){
}

class EndPosPicking:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		if(wxGetApp().m_digitizing->m_doing_a_main_loop)
		{
			wxGetApp().ExitMainLoop();
		}
		else{
			wxMessageBox(_("Error! The 'Stop Picking' button shouldn't have been available!"));
		}
	}
	const wxChar* GetTitle(){return _("Stop Picking");}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + _T("/bitmaps/endpospick.png"), wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const wxChar* GetToolTip(){return _("Finish picking");}
};
wxBitmap* EndPosPicking::m_bitmap = NULL;

static EndPosPicking end_pos_picking;

void DigitizeMode::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	if(m_doing_a_main_loop)t_list->push_back(&end_pos_picking);
}

void DigitizeMode::SetOnlyCoords(HeeksObj* object, bool onoff){
	if(onoff)m_only_coords_set.insert(object);
	else m_only_coords_set.erase(object);
}

bool DigitizeMode::OnlyCoords(HeeksObj* object){
	if(m_only_coords_set.find(object) != m_only_coords_set.end())return true;
	return false;
}
