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

DigitizeMode::DigitizeMode(){
	point_or_window = new PointOrWindow(false);
	position_found = gp_Pnt(0, 0, 0);
	digitize_type_found = DigitizeNoItemType;
	m_doing_a_main_loop = false;
}

DigitizeMode::~DigitizeMode(void){
	delete point_or_window;
}

void DigitizeMode::OnMouse( wxMouseEvent& event ){
	if(event.LeftDown()){
		point_or_window->OnMouse(event);
		lbutton_type_found = digitize(wxPoint(event.GetX(), event.GetY()));
		lbutton_position = position_found;
	}
	else if(event.LeftUp()){
		if(lbutton_type_found != DigitizeNoItemType){
			position_found = lbutton_position;
			digitize_type_found = lbutton_type_found;
			if(m_doing_a_main_loop){
				wxGetApp().ExitMainLoop();
			}
		}
	}
	else if(event.Moving()){
		digitize(wxPoint(event.GetX(), event.GetY()));
		point_or_window->OnMouse(event);
		if(m_doing_a_main_loop)wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
	}
}

static gp_Trsf global_matrix_relative_to_screen;

static const gp_Trsf& digitizing_matrix(bool calculate = false){
	if(calculate){
		if(wxGetApp().digitize_screen){
			gp_Trsf mat = wxGetApp().digitizing_matrix;
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

class digitize_comparer{
public:
	digitize_comparer(gp_Pnt vt, DigitizeType t){v = vt; type = t;}
	gp_Pnt v;
	DigitizeType type;
	int importance();
};

int digitize_comparer::importance(){
	switch(type){
	case DigitizeEndofType:
		return 10;

	case DigitizeIntersType:
		return 5;

	case DigitizeMidpointType:
		return 7;

	case DigitizeCentreType:
		return 7;

	case DigitizeNearestType:
		return 4;

	default:
		return 0;
	}
}
	
DigitizeType DigitizeMode::digitize1(const wxPoint &input_point, gp_Pnt &point, gp_Pnt &closest_point){
	gp_Lin ray = wxGetApp().m_frame->m_graphics->m_view_point.SightLine(input_point);
	std::list<digitize_comparer> compare_list;
	MarkedObjectManyOfSame marked_object;
	if(wxGetApp().digitize_end || wxGetApp().digitize_inters || wxGetApp().digitize_centre || wxGetApp().digitize_midpoint || wxGetApp().digitize_nearest){
		point_or_window->SetWithPoint(input_point);
		wxGetApp().m_marked_list->ignore_coords_only = true;
		wxGetApp().m_marked_list->ObjectsInWindow(point_or_window->box_chosen, &marked_object);
		wxGetApp().m_marked_list->ignore_coords_only = false;
	}
	if(wxGetApp().digitize_end){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				double pos[3];
				std::list<double> vl;
				std::list<double>::iterator MovePosIt;
				object->GetGripperPositions(&vl, true);
				for(MovePosIt = vl.begin(); MovePosIt != vl.end(); MovePosIt++){
					MovePosIt++;
					if(MovePosIt == vl.end())break;
					pos[0] = *MovePosIt;
					MovePosIt++;
					if(MovePosIt == vl.end())break;
					pos[1] = *MovePosIt;
					MovePosIt++;
					if(MovePosIt == vl.end())break;
					pos[2] = *MovePosIt;
					compare_list.push_back(digitize_comparer(make_point(pos), DigitizeEndofType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(wxGetApp().digitize_midpoint){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				double p[3];
				if(object->GetMidPoint(p)){
					compare_list.push_back(digitize_comparer(make_point(p), DigitizeMidpointType));
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
					compare_list.push_back(digitize_comparer(make_point(p), DigitizeNearestType));
				}
				object = marked_object.Increment();
			}
		}
	}
	double min_dist = -1;
	double best_dp = 0;
	digitize_comparer *best_digitize_comparer = NULL;
	if(compare_list.size() >0){
		std::list<digitize_comparer>::iterator It;
		double dist;
		double dp;
		for(It = compare_list.begin(); It != compare_list.end(); It++){
			digitize_comparer *this_digitize_comparer = &(*It);
			dist = ray.Distance(this_digitize_comparer->v);
			dp = gp_Vec(ray.Direction()) * gp_Vec(this_digitize_comparer->v.XYZ()) - gp_Vec(ray.Direction()) * gp_Vec(ray.Location().XYZ());
			if(dist * wxGetApp().GetPixelScale() < 2)dist = 2/wxGetApp().GetPixelScale();
			if(dist * wxGetApp().GetPixelScale()>10)continue;
			bool use_this = false;
			if(best_digitize_comparer == NULL)use_this = true;
			else if(dist<min_dist)use_this = true;
			else{
				bool same_dist = fabs(dist - min_dist)<wxGetApp().m_geom_tol;
				if(same_dist){
					double rel_dp = dp - best_dp;
					if(fabs(rel_dp)<2/wxGetApp().GetPixelScale() + wxGetApp().m_geom_tol)rel_dp = 0;
					bool less_dp = rel_dp < -wxGetApp().m_geom_tol;
					if(less_dp)use_this = true;
					else{
						bool same_dp = fabs(rel_dp)<wxGetApp().m_geom_tol;
						if(same_dp){
							if(this_digitize_comparer->importance() > best_digitize_comparer->importance())use_this = true;
						}
					}
				}
			}
			if(use_this){
				min_dist = dist;
				best_digitize_comparer = this_digitize_comparer;
				best_dp = dp;
			}
		}
	}
	if(wxGetApp().digitize_centre && (min_dist == -1 || min_dist * wxGetApp().GetPixelScale()>5)){
		gp_Pnt pos;
		for(HeeksObj* object = marked_object.GetFirstOfEverything(); object != NULL; object = marked_object.Increment()){
			double p[3];
			if(object->GetCentrePoint(p)){
				compare_list.push_back(digitize_comparer(make_point(p), DigitizeCentreType));
				best_digitize_comparer = &(compare_list.back());
				break;
			}
		}
	}
	DigitizeType type_to_return = DigitizeNoItemType;
	if(best_digitize_comparer){
		point = best_digitize_comparer->v;
		type_to_return = best_digitize_comparer->type;
	}
	else if(wxGetApp().digitize_coords){
		type_to_return = Digitize(ray, point);
	}
	
	return type_to_return;
}

DigitizeType DigitizeMode::Digitize(const gp_Lin &ray, gp_Pnt &point){
	gp_Pln pl(gp_Pnt(0, 0, 0), gp_Vec(0, 0, 1));
	pl.Transform(digitizing_matrix(true));
	std::list<gp_Pnt> points;
	intersect(ray, pl, points);
	if(points.size() == 0){
		pl = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, -1, 0));
		intersect(ray, pl, points);
		if(points.size()>0)point = points.front();
		else{
			pl = gp_Pln(gp_Pnt(0, 0, 0), gp_Vec(1, 0, 0));
			intersect(ray, pl, points);
			if(points.size()>0)point = points.front();
		}
		return DigitizeNoItemType;
	}
	point = points.front();

	if(wxGetApp().draw_to_grid){
		gp_Vec plane_vx = gp_Vec(1, 0, 0).Transformed(digitizing_matrix());
		gp_Vec plane_vy = gp_Vec(0, 1, 0).Transformed(digitizing_matrix());
		gp_Pnt datum = gp_Pnt(0, 0, 0).Transformed(digitizing_matrix());

		double a = gp_Vec(datum.XYZ()) * plane_vx;
		double b = gp_Vec(point.XYZ()) * plane_vx;
		double c = b - a;
		double extra1 = c > -0.00000001 ? 0.5:-0.5;
		c = (int)(c / wxGetApp().digitizing_grid + extra1) * wxGetApp().digitizing_grid;

		double datum_dotp_y = gp_Vec(datum.XYZ()) * plane_vy;
		double rp_dotp_y = gp_Vec(point.XYZ()) * plane_vy;
		double d = rp_dotp_y - datum_dotp_y;
		double extra2 = d > -0.00000001 ? 0.5:-0.5;
		d = (int)(d / wxGetApp().digitizing_grid + extra2) * wxGetApp().digitizing_grid;

		point = datum.XYZ() + plane_vx.XYZ() * c + plane_vy.XYZ() * d;

	}

	return DigitizeCoordsType;
}

DigitizeType DigitizeMode::digitize(const wxPoint &point){
	gp_Pnt closest_point;
	digitize_type_found = digitize1(point, position_found, closest_point);
	return digitize_type_found;
}

void DigitizeMode::OnFrontRender(){
	point_or_window->OnFrontRender();
}

void on_intersection(bool onoff){
	wxGetApp().digitize_inters = onoff;
}

void on_centre(bool onoff){
	wxGetApp().digitize_centre = onoff;
}

void on_end_of(bool onoff){
	wxGetApp().digitize_end = onoff;
}

void on_mid_point(bool onoff){
	wxGetApp().digitize_midpoint = onoff;
}

void on_nearest(bool onoff){
	wxGetApp().digitize_nearest = onoff;
}

void on_coords(bool onoff){
	wxGetApp().digitize_coords = onoff;
}

void on_relative(bool onoff){
	wxGetApp().digitize_screen = onoff;
}

static void set_x(double value){wxGetApp().m_digitizing->position_found.SetX(value); wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();}
static void set_y(double value){wxGetApp().m_digitizing->position_found.SetY(value); wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();}
static void set_z(double value){wxGetApp().m_digitizing->position_found.SetZ(value); wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();}

void DigitizeMode::GetProperties(std::list<Property *> *list){
	if(m_doing_a_main_loop)
	{
		// set the title for picking a position
		list->push_back(new PropertyString("Position Picking...", m_prompt_when_doing_a_main_loop.c_str()));
		list->push_back(new PropertyDouble("X", position_found.X(), set_x));
		list->push_back(new PropertyDouble("Y", position_found.Y(), set_y));
		list->push_back(new PropertyDouble("Z", position_found.Z(), set_z));
	}
}

void DigitizeMode::GetOptions(std::list<Property *> *list){
	PropertyList* plist = new PropertyList("digitizing");
	plist->m_list.push_back(new PropertyCheck("end", wxGetApp().digitize_end, on_end_of));
	plist->m_list.push_back(new PropertyCheck("intersection", wxGetApp().digitize_inters, on_intersection));
	plist->m_list.push_back(new PropertyCheck("centre", wxGetApp().digitize_centre, on_centre));
	plist->m_list.push_back(new PropertyCheck("midpoint", wxGetApp().digitize_midpoint, on_mid_point));
	plist->m_list.push_back(new PropertyCheck("nearest", wxGetApp().digitize_nearest, on_nearest));
	plist->m_list.push_back(new PropertyCheck("coordinates", wxGetApp().digitize_coords, on_coords));
	plist->m_list.push_back(new PropertyCheck("screen", wxGetApp().digitize_screen, on_relative));
	list->push_back(plist);
}

class EndPosPicking:public Tool{
private:
	static wxBitmap* m_bitmap;

public:
	void Run(){
		if(wxGetApp().m_select_mode->m_doing_a_main_loop)
		{
			wxGetApp().ExitMainLoop();
		}
		else{
			wxMessageBox("Error! The \"Stop Picking\" button shouldn't have been available!");
		}
	}
	const char* GetTitle(){return "Stop Picking";}
	wxBitmap* Bitmap()
	{
		if(m_bitmap == NULL)
		{
			wxString exe_folder = wxGetApp().GetExeFolder();
			m_bitmap = new wxBitmap(exe_folder + "/bitmaps/endpospick.png", wxBITMAP_TYPE_PNG);
		}
		return m_bitmap;
	}
	const char* GetToolTip(){return "Finish picking";}
};
wxBitmap* EndPosPicking::m_bitmap = NULL;


void DigitizeMode::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	if(m_doing_a_main_loop)t_list->push_back(new EndPosPicking);
}

void DigitizeMode::SetOnlyCoords(HeeksObj* object, bool onoff){
	if(onoff)m_only_coords_set.insert(object);
	else m_only_coords_set.erase(object);
}

bool DigitizeMode::OnlyCoords(HeeksObj* object){
	if(m_only_coords_set.find(object) != m_only_coords_set.end())return true;
	return false;
}
