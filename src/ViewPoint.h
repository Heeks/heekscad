// ViewPoint.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class CViewport;

class CViewPoint{
private:
	wxPoint m_initial_point;
	double m_initial_pixel_scale;
	bool m_perspective;
	CViewport* m_viewport;

	void SetProjection2(bool use_depth_testing);
	int ChooseBestPlane(int plane)const;
	
public:
	bool m_section;
	gp_Pnt m_lens_point;
	gp_Pnt m_target_point;
	gp_Vec m_vertical;
	double m_pixel_scale;  // not valid for perspective
	double m_view_angle;  // only valid for perspective
	double m_projm[16], m_modelm[16]; 
	int m_window_rect[4];
	bool m_matrix_valid;
	double m_near_plane;
	double m_far_plane;
	CBox m_extra_depth_box;
	CBox m_extra_view_box;

	CViewPoint(CViewport* viewport);
	~CViewPoint(void);
	CViewPoint(const CViewPoint &c);

	const CViewPoint& operator=(const CViewPoint &c);

	const gp_Vec rightwards_vector(void)const{return gp_Vec(m_lens_point, m_target_point) ^ m_vertical;}
	const gp_Vec forwards_vector(void)const{return gp_Vec(m_lens_point, m_target_point);}
	void Turn(double ang_x, double ang_y);
	void Turn(wxPoint point_diff);
	void TurnVertical(double ang_x, double ang_y);
	void TurnVertical(wxPoint point_diff);
	void Shift(const gp_Vec &tv);
	void Shift(const wxPoint &point_diff, const wxPoint &point);
	void Scale(double multiplier, bool use_initial_pixel_scale = false);
	void Scale(const wxPoint &point, bool reversed = false);
	void Twist(double angle);
	void Twist(wxPoint start, wxPoint point_diff);
	void SetViewport()const;
	void SetProjection(bool use_depth_testing);
	void SetPickProjection(wxRect &pick_box);
	void SetModelview(void);
	void SetView(const gp_Vec &unity, const gp_Vec &unitz);
	gp_Pnt glUnproject(const gp_Pnt &v)const;
	gp_Pnt glProject(const gp_Pnt &v)const;
	void SetPolygonOffset(void)const;
	void WindowMag(wxRect &window_box);
	void SetViewAroundAllObjects();
	void SetStartMousePoint(const wxPoint &point){m_initial_pixel_scale = m_pixel_scale; m_initial_point = point;}
	gp_Lin SightLine(const wxPoint &point);
	int GetTwoAxes(gp_Vec& vx, gp_Vec& vy, bool flattened_onto_screen, int plane)const;
	void Set90PlaneDrawMatrix(gp_Trsf &mat)const;
	void SetPerspective(bool perspective);
	bool GetPerspective(){return m_perspective;}
};
