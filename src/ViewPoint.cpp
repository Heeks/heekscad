// ViewPoint.cpp
#include "stdafx.h"
#include "ViewPoint.h"
#include "SelectMode.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"

CViewPoint::CViewPoint(void){
	m_lens_point = gp_Pnt(0, 0, 200);
	m_target_point = gp_Pnt(0, 0, 0);
	m_vertical = gp_Vec(0, 1, 0);
	pixel_scale = 10;
	m_section = false;
	m_matrix_valid = false;
}

CViewPoint::~CViewPoint(void){
}

CViewPoint::CViewPoint(const CViewPoint &c){
	m_matrix_valid = false;
	operator=(c);
}

const CViewPoint& CViewPoint::operator=(const CViewPoint &c){
	m_lens_point = c.m_lens_point;
	m_target_point = c.m_target_point;
	m_vertical = c.m_vertical;
	pixel_scale = c.pixel_scale;
	m_section = c.m_section;
	memcpy(m_projm, c.m_projm, 16*sizeof(double));
	memcpy(m_modelm, c.m_modelm, 16*sizeof(double));
	memcpy(m_window_rect, c.m_window_rect, 4*sizeof(int));
	m_matrix_valid = c.m_matrix_valid;
	return *this;
}

void CViewPoint::Turn(wxPoint point_diff){
	if(point_diff.x > 100)point_diff.x = 100;
	else if(point_diff.x < -100)point_diff.x = -100;
	if(point_diff.y > 100)point_diff.y = 100;
	else if(point_diff.y < -100)point_diff.y = -100;
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	double c=(size.GetWidth()+size.GetHeight())/20;
	Turn(point_diff.x/c, point_diff.y/c);
}

void CViewPoint::Turn(double ang_x, double ang_y){
	gp_Vec f(m_lens_point, m_target_point);
	double fl = f.Magnitude();
	gp_Vec uu = m_vertical.Normalized();
	gp_Vec r = (f ^ uu).Normalized();
	m_lens_point = m_lens_point.XYZ() - r.XYZ() * sin(ang_x)*fl;
	m_lens_point = m_lens_point.XYZ() + f.XYZ()*(1-cos(ang_x));
	f = f.XYZ() + r.XYZ() * sin(ang_x) * fl;
	f = f.XYZ() - f.XYZ() * (1-cos(ang_x));
	r = (f ^ uu).Normalized();
	m_lens_point = m_lens_point.XYZ() + uu.XYZ() * sin(ang_y)*fl;
	m_lens_point = m_lens_point.XYZ() + f.XYZ() * (1-cos(ang_y));
	m_vertical = m_vertical.XYZ() + f.XYZ() * sin(ang_y)/fl;
	m_vertical = m_vertical.XYZ() - uu.XYZ() * (1-cos(ang_y));
}

void CViewPoint::TurnVertical(wxPoint point_diff){
	if(point_diff.x > 100)point_diff.x = 100;
	else if(point_diff.x < -100)point_diff.x = -100;
	if(point_diff.y > 100)point_diff.y = 100;
	else if(point_diff.y < -100)point_diff.y = -100;
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	double c=(size.GetWidth()+size.GetHeight())/20;
	TurnVertical(point_diff.x/c, point_diff.y/c);
}

void CViewPoint::TurnVertical(double ang_x, double ang_y){
	gp_Vec f = m_target_point.XYZ() - m_lens_point.XYZ();
	gp_Vec uu = m_vertical.Normalized();
	gp_Vec r = (f ^ uu).Normalized();
	gp_Pnt temp_target(m_target_point.X(), m_target_point.Y(), 0);
	double dist = gp_Pnt(m_lens_point.X(), m_lens_point.Y(), 0).Distance(temp_target);
	double a_old = atan2(m_lens_point.Y() - m_target_point.Y(), m_lens_point.X() - m_target_point.X());
	double a_new = a_old - ang_x;
	m_lens_point.SetX( m_target_point.X() + dist * cos(a_new));
	m_lens_point.SetY( m_target_point.Y() + dist * sin(a_new));
	gp_Pnt temp_vertical(m_vertical.X(), m_vertical.Y(), 0);
	temp_target.SetX(0);
	temp_target.SetY(0);
	dist = temp_vertical.Distance(temp_target);
	a_old = atan2(temp_vertical.Y() - temp_target.Y(), temp_vertical.X() - temp_target.X());
	a_new = a_old - ang_x;
	m_vertical.SetX(temp_target.X() + dist * cos(a_new));
	m_vertical.SetY(temp_target.Y() + dist * sin(a_new));
	f = make_vector(m_lens_point, m_target_point);
	uu = m_vertical.Normalized();
	r = (f ^ uu).Normalized();
	if(ang_y>Pi/2)ang_y = Pi/2;
	else if(ang_y<-Pi/2)ang_y = -Pi/2;
	double fl = f.Magnitude();
	double div_magn = 1/fl;
	bool beyond_vertical = false;
	double z_increase = sin(ang_y)*f.Z()*div_magn - (1-cos(ang_y))*uu.Z();
	if((m_vertical.Z() + z_increase )<0 && z_increase<0)beyond_vertical = true;
	double angle = ang_y/2;
	m_lens_point = m_lens_point.XYZ() + uu.XYZ() * fl * sin(angle) + f.XYZ() * (1-cos(angle));
	m_vertical = m_vertical.XYZ() + f.XYZ() * (sin(angle) * div_magn) + uu.XYZ() * ( cos(angle) - 1);
	if(beyond_vertical){
		m_vertical.SetZ(0);
		m_vertical.Normalize();
		f = (m_vertical ^ r) * fl;
		m_lens_point = m_target_point.XYZ() - f.XYZ();
	}
}

void CViewPoint::Shift(const gp_Vec &tv){
	gp_Vec r = rightwards_vector().Normalized();
	gp_Vec f = forwards_vector().Normalized();
	gp_Vec u = m_vertical.Normalized();
	gp_Vec new_vector = r.XYZ() * tv.X() + f.XYZ() * tv.Z() + u.XYZ() * tv.Y();
	m_lens_point = m_lens_point.XYZ() + new_vector.XYZ();
	m_target_point = m_target_point.XYZ() + new_vector.XYZ();
}

void CViewPoint::Shift(const wxPoint &point_diff, const wxPoint &point){
	double div_x = (double)(point_diff.x)/pixel_scale;
	double div_y = (double)(point_diff.y)/pixel_scale;
	gp_Vec f = m_target_point.XYZ() - m_lens_point.XYZ();
	gp_Vec uu = m_vertical.Normalized();
	gp_Vec r = (f ^ uu).Normalized();
	m_target_point = m_target_point.XYZ() - r.XYZ() * div_x;
	m_lens_point = m_lens_point.XYZ() - r.XYZ() * div_x;
	m_target_point = m_target_point.XYZ() + uu.XYZ() * div_y;
	m_lens_point = m_lens_point.XYZ() + uu.XYZ() * div_y;
}

void CViewPoint::WindowMag(wxRect &window_box){
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	double width_ratio=fabs((double)(window_box.width)/(double)(size.GetWidth()));
	double height_ratio=fabs((double)(window_box.height)/(double)(size.GetHeight()));
	if(height_ratio>width_ratio)width_ratio=height_ratio;
	double old_xcen = (double)(size.GetWidth())*0.5;
	double old_ycen = (double)(size.GetHeight())*0.5;
	double new_xcen = (double)window_box.x+(double)(window_box.width)*0.5;
	double new_ycen = (double)window_box.y+(double)(window_box.height)*0.5;
	double move_xcen = new_xcen-old_xcen;
	double move_ycen = new_ycen-old_ycen;
	gp_Vec f = m_target_point.XYZ() - m_lens_point.XYZ();
	gp_Vec uu = m_vertical.Normalized();
	gp_Vec r = (f ^ uu).Normalized();
	m_target_point = m_target_point.XYZ() + r.XYZ() * move_xcen/pixel_scale;
	m_lens_point = m_lens_point.XYZ() + r.XYZ() * move_xcen/pixel_scale;
	m_target_point = m_target_point.XYZ() + uu.XYZ() * move_ycen/pixel_scale;
	m_lens_point = m_lens_point.XYZ() + uu.XYZ() * move_ycen/pixel_scale;
	pixel_scale/=width_ratio;
}

void CViewPoint::Scale(double multiplier, bool use_initial_pixel_scale){
	bool increasing=(multiplier>0);
	if(!increasing)multiplier = -multiplier;
	multiplier = 1 - multiplier;
	if(multiplier<0.00001)multiplier=0.00001;
	if(increasing)multiplier = 1/multiplier;
	if(multiplier< 0.1)multiplier = 0.1;
	if(use_initial_pixel_scale)pixel_scale = m_initial_pixel_scale;
	pixel_scale *= multiplier;
	if(pixel_scale > 1000000)pixel_scale = 1000000;
	if(pixel_scale < 0.000001)pixel_scale = 0.000001;
}

void CViewPoint::Scale(const wxPoint &point){
	double mouse_ydiff = point.y - m_initial_point.y;
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	double multiplier=(mouse_ydiff)/((double)(size.GetHeight()));
	Scale(multiplier, true);
}	

void CViewPoint::Twist(double angle){
	gp_Vec f = m_target_point.XYZ() - m_lens_point.XYZ();
	gp_Vec uu = m_vertical.Normalized();
	gp_Vec r = (f ^ uu).Normalized();
	m_vertical = r.XYZ() * -sin(angle) + uu.XYZ() * cos(angle);
}

void CViewPoint::Twist(wxPoint start, wxPoint point_diff){
	gp_Vec f = m_target_point.XYZ() - m_lens_point.XYZ();
	gp_Vec uu = m_vertical.Normalized();
	gp_Vec r = (f ^ uu).Normalized();
	gp_Pnt screen_target = glProject(m_target_point);
	gp_Pnt screen_start((double)start.x, (double)start.y, 0);
	gp_Pnt screen_end((double)(start.x + point_diff.x), (double)(start.y + point_diff.y), 0);
	double old_angle = atan2(screen_start.Y() - screen_target.Y(), screen_start.X() - screen_target.X());
	double angle = atan2(screen_end.Y() - screen_target.Y(), screen_end.X() - screen_target.X());
	m_vertical = r.XYZ() * -sin(angle - old_angle) + uu.XYZ() * cos(angle - old_angle);
}

void CViewPoint::SetProjection2(bool use_depth_testing)const{
	double near_plane;
	double far_plane;
	double rad;
	CBox box;
	if(use_depth_testing)wxGetApp().GetBox(box);
	if(!use_depth_testing){
		near_plane = 0;
		far_plane = 100000000;
		rad = 20000000;
	}
	else if(!box.m_valid){
		near_plane = 0.2;
		far_plane = 2000;
		rad = 1000;
	}
	else{
		double boxc[3];
		box.Centre(boxc);
		rad = box.Radius();
		gp_Vec to_centre_of_box = make_vector(m_lens_point, make_point(boxc));
		gp_Vec f = forwards_vector();
		f.Normalize();
		double distance =to_centre_of_box*f;
		if(m_section)rad /= 100;
		near_plane = distance - rad;
		far_plane = distance + rad;
	}
	{
		wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
		double w = size.GetWidth()/pixel_scale;
		double h = size.GetHeight()/pixel_scale;
		double s = sqrt(w*w + h*h);
		near_plane -= s/2;
		far_plane += s/2;
	}
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	double hw = (double)(size.GetWidth())/2;
	double hh = (double)(size.GetHeight())/2;
	glOrtho((-0.5 - hw)/pixel_scale, (hw+0.5)/pixel_scale, (-0.5-hh)/pixel_scale, (0.5+hh)/pixel_scale, (GLfloat)near_plane, (GLfloat)far_plane);
}

void CViewPoint::SetProjection(bool use_depth_testing){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	SetProjection2(use_depth_testing);
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	m_window_rect[0] = 0;
	m_window_rect[1] = 0;
	m_window_rect[2]=size.GetWidth();
	m_window_rect[3]=size.GetHeight();
	glGetDoublev (GL_PROJECTION_MATRIX, m_projm);
	m_matrix_valid = true;
}

void CViewPoint::SetPickProjection(wxRect &pick_box)const{
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	int vp[4] ={0, 0, size.GetWidth(), size.GetHeight()};
	double box_width = pick_box.width;
	double box_height = pick_box.height;
	double x_centre = (double)pick_box.x + box_width/2;
	double y_centre = (double)pick_box.y + box_height/2;
	if(box_width == 0)box_width = 1;
	if(box_height == 0)box_height = 1;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix(x_centre, y_centre, abs(box_width), abs(box_height), vp);
	SetProjection2(true);
}

void CViewPoint::SetModelview(void){
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_lens_point.X(), m_lens_point.Y(), m_lens_point.Z(), m_target_point.X(), m_target_point.Y(), m_target_point.Z(), m_vertical.X(), m_vertical.Y(), m_vertical.Z());
	glGetDoublev (GL_MODELVIEW_MATRIX, m_modelm);
}

void CViewPoint::SetViewport(void)const{
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	glViewport(0, 0, size.GetWidth(), size.GetHeight());
}

void CViewPoint::SetView(gp_Vec &unity, gp_Vec &unitz){
	m_target_point = gp_Pnt(0, 0, 0);
	m_lens_point = m_target_point.XYZ() + unitz.XYZ();
	m_vertical = unity;
	pixel_scale = 10;
	SetViewAroundAllObjects();
}

gp_Pnt CViewPoint::glUnproject(const gp_Pnt &v)const{
	if(!m_matrix_valid)return gp_Pnt(0, 0, 0);
	double x, y, z;
	gluUnProject(v.X(), v.Y(), v.Z(), m_modelm, m_projm, m_window_rect, &x, &y, &z);
	gp_Pnt temp(x, y, z);
	return temp;
}

gp_Pnt CViewPoint::glProject(const gp_Pnt &v)const{
	if(!m_matrix_valid)return gp_Pnt(0, 0, 0);
	double x, y, z;
	gluProject(v.X(), v.Y(), v.Z(), m_modelm, m_projm, m_window_rect, &x, &y, &z);
	gp_Pnt temp(x, y, z);
	return temp;
}

void CViewPoint::SetPolygonOffset(void)const{
	glPolygonOffset(1.0, 1.0);
}

void CViewPoint::SetViewAroundAllObjects(){
	CBox box;
	wxGetApp().GetBox(box);
	if(!box.m_valid)return;
	gp_Vec r = rightwards_vector().Normalized();
	CBox window;
	wxSize size = wxGetApp().m_frame->m_graphics->GetClientSize();
	double width_ratio = (double)(size.GetWidth())/(double)(size.GetHeight());
	int width = size.GetWidth();
	int height = size.GetHeight();
	for(int i = 0; i<8; i++){
		double p[3];
		box.vert(i, p);
		double x = r * make_vector(p);
		double y = m_vertical * make_vector(p);
		window.Insert(x, y, 0);
	}
	gp_Vec uf = make_vector(m_lens_point, m_target_point).Normalized();
	double boxc[3];
	box.Centre(boxc);
	double cx = r * make_vector(boxc);
	double cy = m_vertical * make_vector(boxc);
	double m = fabs((window.m_x[3] - cx)/(0.255341921221036 * width_ratio));
	double y_dist =  fabs((window.m_x[4] - cy)/0.255341921221036);
	if(y_dist>m)m = y_dist;
	m += fabs(uf * make_vector(boxc));
	m_target_point = make_point(boxc);
	m_lens_point = m_target_point.XYZ() - (uf.XYZ() * m);
	double Width = window.Width();
	double Height = window.Height();
	double pw = width - 6;
	double ph = height - 6;
	if(Width<0.00001)Width = 0.00001;
	if(Height<0.00001)Height = 0.00001;
	if(pw<0.00001)pw = 0.00001;
	if(ph<0.00001)ph = 0.00001;
	double px = pw/Width;
	double py = ph/Height;
	if(px<py)pixel_scale = px;
	else pixel_scale = py;
	m_section = false;
}

gp_Lin CViewPoint::SightLine(const wxPoint &point){
	gp_Pnt screen_point(point.x, wxGetApp().m_frame->m_graphics->GetSize().GetHeight()-point.y, 0);
	gp_Pnt s = glUnproject(screen_point);
	screen_point.SetZ(1);
	gp_Pnt e = glUnproject(screen_point);
	gp_Vec dir(s, e);
	return gp_Lin(s, dir);
}

int CViewPoint::ChooseBestPlane(int plane)const{
	gp_Vec f = forwards_vector();
	double dp[3];
	gp_Trsf orimat = wxGetApp().digitizing_matrix;
	dp[0] = gp_Vec(0, 0, 1).Transformed(orimat) * f;
	dp[1] = gp_Vec(0, 1, 0).Transformed(orimat) * f;
	dp[2] = gp_Vec(1, 0, 0).Transformed(orimat) * f;
	double best_dp = 0.0;
	int best_mode = -1;
	double second_best_dp = 0.0;
	int second_best_mode = -1;
	double third_best_dp;
	int third_best_mode = -1;
	for(int i = 0; i<3; i++){
		if(best_mode == -1){
			best_mode = i;
			best_dp = fabs(dp[i]);
		}
		else{
			if(fabs(dp[i])>best_dp){
				third_best_dp = second_best_dp;
				third_best_mode = second_best_mode;
				second_best_dp = best_dp;
				second_best_mode = best_mode;
				best_mode = i;
				best_dp = fabs(dp[i]);
			}
			else{
				if(second_best_mode == -1){
					second_best_mode = i;
					second_best_dp = fabs(dp[i]);
				}
				else{
					if(fabs(dp[i])>second_best_dp){
						third_best_dp = second_best_dp;
						third_best_mode = second_best_mode;
						second_best_dp = fabs(dp[i]);
						second_best_mode = i;
					}
					else{
						third_best_dp = fabs(dp[i]);
						third_best_mode = i;
					}
				}
			}
		}
	}
	switch(plane){
		case 0:
			return best_mode;
		case 1:
			return second_best_mode;
		default:
			return third_best_mode;
	}
}

int CViewPoint::GetTwoAxes(gp_Vec& vx, gp_Vec& vy, bool flattened_onto_screen, int plane)const{
	int plane_mode = ChooseBestPlane(plane);
	gp_Trsf orimat = wxGetApp().digitizing_matrix;
	switch(plane_mode){
	case 0:
		vx = gp_Vec(1, 0, 0).Transformed(orimat);
		vy = gp_Vec(0, 1, 0).Transformed(orimat);
		break;

	case 1:
		vx = gp_Vec(1, 0, 0).Transformed(orimat);
		vy = gp_Vec(0, 0, 1).Transformed(orimat);
		break;

	case 2:
		vx = gp_Vec(0, 1, 0).Transformed(orimat);
		vy = gp_Vec(0, 0, 1).Transformed(orimat);
		break;
	}
	
	if(flattened_onto_screen){
		gp_Vec f = forwards_vector().Normalized();
		vx = gp_Vec(vx.XYZ() - (f * (f * vx)).XYZ()).Normalized();
		vy = gp_Vec(vy.XYZ() - (f * (f * vy)).XYZ()).Normalized();
		gp_Vec r = rightwards_vector();
		if(fabs(vy * r) > fabs(vx * r)){
			gp_Vec temp = vx;
			vx = vy;
			vy = temp;
		}
	}
	return plane_mode;
}

void CViewPoint::Set90PlaneDrawMatrix(gp_Trsf &mat)const{
	int plane = ChooseBestPlane(0);
	mat = wxGetApp().digitizing_matrix;
	switch(plane){
	case 1:
		mat = make_matrix(gp_Pnt(0, 0, 0).Transformed(mat), gp_Vec(1, 0, 0).Transformed(mat), gp_Vec(0, 0, 1).Transformed(mat));
		break;
		
	case 2:
		mat = make_matrix(gp_Pnt(0, 0, 0).Transformed(mat), gp_Vec(0, 1, 0).Transformed(mat), gp_Vec(0, 0, 1).Transformed(mat));
		break;
	}
}
