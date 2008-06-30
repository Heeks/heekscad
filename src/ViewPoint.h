// ViewPoint.h

#pragma once

class CViewPoint{
private:
	wxPoint m_initial_point;
	double m_initial_pixel_scale;

	void SetProjection2(bool use_depth_testing)const;
	int ChooseBestPlane(int plane)const;
	
public:
	bool m_section;
	gp_Pnt m_lens_point;
	gp_Pnt m_target_point;
	gp_Vec m_vertical;
	double pixel_scale;
	double m_projm[16], m_modelm[16]; 
	int m_window_rect[4];
	bool m_matrix_valid;

	CViewPoint(void);
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
	void Scale(const wxPoint &point);
	void Twist(double angle);
	void Twist(wxPoint start, wxPoint point_diff);
	void SetViewport(void)const;
	void SetProjection(bool use_depth_testing);
	void SetPickProjection(wxRect &pick_box)const;
	void SetModelview(void);
	void SetView(gp_Vec &unity, gp_Vec &unitz);
	gp_Pnt glUnproject(const gp_Pnt &v)const;
	gp_Pnt glProject(const gp_Pnt &v)const;
	void SetPolygonOffset(void)const;
	void WindowMag(wxRect &window_box);
	void SetViewAroundAllObjects();
	void SetStartMousePoint(const wxPoint &point){m_initial_pixel_scale = pixel_scale; m_initial_point = point;}
	gp_Lin SightLine(const wxPoint &point);
	int GetTwoAxes(gp_Vec& vx, gp_Vec& vy, bool flattened_onto_screen, int plane)const;
	void Set90PlaneDrawMatrix(gp_Trsf &mat)const;
};
