// HGear.cpp
// Copyright (c) 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HGear.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyInt.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyLength.h"
#include "Gripper.h"
#include "HLine.h"
#include "HArc.h"
#include "HSpline.h"

HGear::HGear(const HGear &o){
	operator=(o);
}

HGear::HGear(){
	m_num_teeth = 12;
	m_module = 1.0;
	m_addendum_offset = 0.0;
	m_addendum_multiplier = 1.0;
	m_dedendum_multiplier = 1.0;
	m_pressure_angle = 0.34906585039886; // 20 degrees
	m_tip_relief = 0.05;
	m_depth = 0.0;
	m_cone_half_angle = 0.0;
	m_angle = 0.0;
}

HGear::~HGear(){
}

const HGear& HGear::operator=(const HGear &o){
	HeeksObj::operator=(o);
	return *this;
}

const wxBitmap &HGear::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/gear.png")));
	return *icon;
}

bool HGear::IsDifferent(HeeksObj* other)
{
	//HGear* gear = (HGear*)other;
	//if(cir->C->m_p.Distance(C->m_p) > wxGetApp().m_geom_tol)
	//	return true;

	return HeeksObj::IsDifferent(other);
}

class PhiAndAngle{
public:
	double phi;
	double angle;
	PhiAndAngle(double Phi, double Angle){phi = Phi; angle = Angle;}
};

void(*callbackfunc_for_point)(const double *p) = NULL;
gp_Trsf mat_for_point;
const HGear* gear_for_point = NULL;
double height_for_point = 0.0;
double pitch_radius = 0.0;
double base_radius = 0.0;
double cone_sin_for_point = 0.0;
double cone_cos_for_point = 0.0;
static double inside_radius = 0.0;
static double outside_radius = 0.0;
static PhiAndAngle outside_phi_and_angle(0.0, 0.0);
static PhiAndAngle inside_phi_and_angle(0.0, 0.0);
static PhiAndAngle tip_relief_phi_and_angle(0.0, 0.0);
static PhiAndAngle middle_phi_and_angle(0.0, 0.0);
static CSketch* sketch_for_gear = NULL;
static std::list<gp_Pnt> spline_points_for_gear;

void point_at_phi(double phi, double &px, double &py)
{
	double x = base_radius * phi;
	double sx = cos(phi) * base_radius;
	double sy = sin(phi) * base_radius;
	px = sx + sin(phi) * x;
	py = sy - cos(phi) * x;
}

PhiAndAngle involute_intersect(double r, double phi, double phi_step)
{
	while(1)
	{
		double px, py;
		point_at_phi(phi, px, py);

        if(sqrt(px*px + py*py) > r)
		{
			if(phi_step > 1e-10)
                return involute_intersect(r, phi-phi_step, phi_step/2);
            return PhiAndAngle(phi, atan2(py, px));
		}
		phi = phi + phi_step;
	}
}

PhiAndAngle involute_intersect(double r)
{
	return involute_intersect(r, 1.0, 1.0);
}

void transform_for_cone_and_depth(gp_Pnt &p)
{
	gp_Vec v(p.XYZ());
	double radius_beyond_surface = v.Magnitude() - pitch_radius;
	v.Normalize();

	double scale = 1.0 - cone_sin_for_point * height_for_point / pitch_radius;

	p = gp_Pnt(v.XYZ() * (scale * pitch_radius + scale * radius_beyond_surface * cone_cos_for_point)
		+ gp_XYZ(0.0, 0.0, height_for_point * cone_cos_for_point + scale * radius_beyond_surface * cone_sin_for_point));
}

void point(double x, double y)
{
	// input - a point as if for a spur gear at 0, 0, 0
	
	// this transforms the point to put it on the cone, for a bevel gear
	// then calls the callback function

	gp_Pnt p(x, y, 0.0);

	transform_for_cone_and_depth(p);

	p.Transform(mat_for_point);
	if(sketch_for_gear)
	{
		spline_points_for_gear.push_back(p);
	}
	
	if(callbackfunc_for_point)
	{
		double pp[3];
		extract(p, pp);
		(*callbackfunc_for_point)(pp);
	}
}

void add_spline()
{
	// add spline
	if(sketch_for_gear == NULL)return;

	if(spline_points_for_gear.size() < 2)return;

	if(spline_points_for_gear.size() == 2)
		sketch_for_gear->Add(new HLine(spline_points_for_gear.front(), spline_points_for_gear.back(), &wxGetApp().current_color), NULL);
	else
		sketch_for_gear->Add(new HSpline(spline_points_for_gear, &wxGetApp().current_color), NULL);

	// clear points, retaining last one
	gp_Pnt back = spline_points_for_gear.back();
	spline_points_for_gear.clear();
	spline_points_for_gear.push_back(back);
}

void involute(double tooth_angle, bool do_reverse)
{
	int steps = 10;
	bool first = true;

	for(int i = do_reverse ? (steps) : 0; do_reverse ? (i>= 0) : (i<=steps); )
	{
		double phi = inside_phi_and_angle.phi + (tip_relief_phi_and_angle.phi - inside_phi_and_angle.phi)*i/steps;

		// calculate point on the first tooth
		double px, py;
		point_at_phi(phi, px, py);
		if(do_reverse)py = -py; // mirror for reverse

		// rotate by tooth angle
		double x = px*cos(tooth_angle) - py*sin(tooth_angle);
		double y = py*cos(tooth_angle) + px*sin(tooth_angle);

		// output the point
		point(x, y);
		if(first)add_spline();
		first = false;

		if(do_reverse)i--;
		else i++;
	}
}

void point_at_rad_and_angle(double r, double angle)
{
	point(cos(angle)*r, sin(angle)*r);
}

enum ClearancePointType{
	CLEARANCE_POINT1_TYPE,
	CLEARANCE_POINT2_TYPE,
};

void get_clearance_points(gp_Pnt &p1_clearance, gp_Pnt &p2_clearance, double tooth_angle, double clearance)
{
	double incremental_angle = 0.5*M_PI/gear_for_point->m_num_teeth - middle_phi_and_angle.angle;
	double angle1 = tooth_angle - (inside_phi_and_angle.angle + incremental_angle);
	double angle2 = tooth_angle + (inside_phi_and_angle.angle + incremental_angle);
	gp_Pnt p1(cos(angle1) * inside_radius, sin(angle1) * inside_radius, 0);
	gp_Pnt p2(cos(angle2) * inside_radius, sin(angle2) * inside_radius, 0);

	gp_Vec v(p1, p2);
	gp_Vec v_in = gp_Vec(0, 0, 1) ^ v;
	v_in.Normalize();

	p1_clearance = gp_Pnt(p1.XYZ() + v_in.XYZ() * clearance);
	p2_clearance = gp_Pnt(p2.XYZ() + v_in.XYZ() * clearance);
}

gp_Pnt mid_point(double tooth_angle, double clearance)
{
	gp_Pnt p1_clearance, p2_clearance;
	get_clearance_points(p1_clearance, p2_clearance, tooth_angle, clearance);
	return gp_Pnt(p1_clearance.XYZ() + (p2_clearance.XYZ() - p1_clearance.XYZ()) * 0.5);
}

double gap_width()
{
	gp_Pnt p1, p2;
	get_clearance_points(p1, p2, 0.0, 0.0);
	return p1.Distance(p2);
}

void clearance_point(double tooth_angle, ClearancePointType point_type, double clearance)
{
	gp_Pnt p1_clearance, p2_clearance;
	get_clearance_points(p1_clearance, p2_clearance, tooth_angle, clearance);

	switch(point_type)
	{
	case CLEARANCE_POINT1_TYPE:
		point(p1_clearance.X(), p1_clearance.Y());	
		break;

	case CLEARANCE_POINT2_TYPE:
		point(p2_clearance.X(), p2_clearance.Y());
		break;
	}

}

void base_point(double tooth_angle, ClearancePointType point_type)
{
	gp_Pnt p1, p2;
	get_clearance_points(p1, p2, tooth_angle, 0.0);

	switch(point_type)
	{
	case CLEARANCE_POINT1_TYPE:
		point(p1.X(), p1.Y());	
		break;
	}
}

void clearance_point1(double tooth_angle, double clearance)
{
	clearance_point(tooth_angle, CLEARANCE_POINT1_TYPE, clearance);
}

void clearance_point2(double tooth_angle, double clearance)
{
	clearance_point(tooth_angle, CLEARANCE_POINT2_TYPE, clearance);
}

void clearance_point_mid(double tooth_angle)
{
	gp_Pnt m = mid_point(tooth_angle, 1.0);
	point(m.X(), m.Y());
}

void base_point1(double tooth_angle)
{
	base_point(tooth_angle, CLEARANCE_POINT1_TYPE);
}

void line_arc_line(double tooth_angle)
{
	double gap = gap_width();
	double radius = gap/3;
	double clearance = gear_for_point->GetClearanceMM();
	double line_length = clearance - radius;
	if(line_length < 0.0)
	{
		line_length = 0.0;
		radius = clearance;
	}

	gp_Pnt p1, p2, p1B, p2B;
	get_clearance_points(p1, p2, tooth_angle, 0.0);
	get_clearance_points(p1B, p2B, tooth_angle, line_length);
	gp_Pnt pm = mid_point(tooth_angle, line_length);
	gp_Circ c(gp_Ax2(pm, gp_Dir(0, 0, -1)), radius);

	if(line_length >= wxGetApp().m_geom_tol)
		sketch_for_gear->Add(new HLine(p1, p1B, &wxGetApp().current_color), NULL);

	double mid_span_line_length = gap - 2*radius;
	if(mid_span_line_length >= wxGetApp().m_geom_tol)
	{
		gp_Vec v(p1, p2);
		v.Normalize();
		gp_Pnt p1C, p2C;
		get_clearance_points(p1C, p2C, tooth_angle, clearance);
		gp_Pnt two_arc_p1 = gp_Pnt(p1C.XYZ() + v.XYZ() * radius);
		gp_Pnt two_arc_p2 = gp_Pnt(two_arc_p1.XYZ() + v.XYZ() * mid_span_line_length);
		gp_Pnt pm1 = gp_Pnt(p1B.XYZ() + v.XYZ() * radius);
		gp_Pnt pm2 = gp_Pnt(pm1.XYZ() + v.XYZ() * mid_span_line_length);
		gp_Circ c1(gp_Ax2(pm1, gp_Dir(0, 0, -1)), radius);
		gp_Circ c2(gp_Ax2(pm2, gp_Dir(0, 0, -1)), radius);
		sketch_for_gear->Add(new HArc(p1B, two_arc_p1, c1, &wxGetApp().current_color), NULL);
		sketch_for_gear->Add(new HLine(two_arc_p1, two_arc_p2, &wxGetApp().current_color), NULL);
		sketch_for_gear->Add(new HArc(two_arc_p2, p2B, c2, &wxGetApp().current_color), NULL);
	}
	else
	{
		sketch_for_gear->Add(new HArc(p1B, p2B, c, &wxGetApp().current_color), NULL);
	}

	if(line_length >= wxGetApp().m_geom_tol)
		sketch_for_gear->Add(new HLine(p2B, p2, &wxGetApp().current_color), NULL);

	spline_points_for_gear.clear();
	spline_points_for_gear.push_back(p2);
}

void tooth(int i, bool want_start_point, bool make_closed_tooth_form)
{
	double tooth_angle = 2*M_PI*i/gear_for_point->m_num_teeth;
	double next_tooth_angle = 2*M_PI*(i+1)/gear_for_point->m_num_teeth;
	// incremental_angle - to space the middle point at a quarter of a cycle
	double incremental_angle = 0.5*M_PI/gear_for_point->m_num_teeth - middle_phi_and_angle.angle;
	double angle1 = tooth_angle - (inside_phi_and_angle.angle + incremental_angle);
	//double angle2 = tooth_angle + (inside_phi_and_angle.angle + incremental_angle);
	double angle3 = tooth_angle + (outside_phi_and_angle.angle + incremental_angle);
	double angle4 = next_tooth_angle - (outside_phi_and_angle.angle + incremental_angle);
	//double angle5 = next_tooth_angle - (inside_phi_and_angle.angle + incremental_angle);

	if(!make_closed_tooth_form && fabs(gear_for_point->GetClearanceMM()) > 0.0000000001)
	{
		add_spline();
		if(sketch_for_gear)
		{			
			line_arc_line(tooth_angle);
		}
		else
		{
			if(i == 0 && want_start_point)base_point1(tooth_angle);
			clearance_point1(tooth_angle, gear_for_point->GetClearanceMM());
			clearance_point2(tooth_angle, gear_for_point->GetClearanceMM());
		}
	}
	else
	{
		if(i==0 && want_start_point)point_at_rad_and_angle(inside_radius, angle1);
	}

	involute(tooth_angle + incremental_angle, false);

	add_spline();

	if(fabs(gear_for_point->m_tip_relief) > 0.00000000001)
	{
		point_at_rad_and_angle(outside_radius, angle3 + (gear_for_point->m_tip_relief/2)/outside_radius);
	add_spline();
		point_at_rad_and_angle(outside_radius, angle4 - (gear_for_point->m_tip_relief/2)/outside_radius);
		add_spline();
	}

	involute(next_tooth_angle - incremental_angle, true);

	add_spline();
}

void HGear::SetSegmentsVariables(void(*callbackfunc)(const double *p))const
{
	callbackfunc_for_point = callbackfunc;
	gear_for_point = this;
	gp_Trsf rotation;
	rotation.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), m_angle * M_PI/180);
	mat_for_point = make_matrix(m_pos.Location(), m_pos.XDirection(), m_pos.YDirection());
	mat_for_point = rotation.Multiplied(mat_for_point);
	cone_sin_for_point = sin(m_cone_half_angle);
	cone_cos_for_point = cos(m_cone_half_angle);

	pitch_radius = (double)(m_module * m_num_teeth)/2;
	inside_radius = pitch_radius - m_dedendum_multiplier*m_module;
	outside_radius = pitch_radius + (m_addendum_multiplier*m_module + m_addendum_offset);
	base_radius = pitch_radius * cos(gear_for_point->m_pressure_angle) / cos(gear_for_point->m_cone_half_angle);

	if(inside_radius < base_radius)inside_radius = base_radius;

	inside_phi_and_angle = involute_intersect(inside_radius);
	outside_phi_and_angle = involute_intersect(outside_radius);
	tip_relief_phi_and_angle = involute_intersect(outside_radius - m_tip_relief);
	middle_phi_and_angle = involute_intersect(pitch_radius);
	spline_points_for_gear.clear();
}

void HGear::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{
	SetSegmentsVariables(callbackfunc);

	for(int i = 0; i<m_num_teeth; i++)
	{
		tooth(i, i==0 && want_start_point, false);
	}
}

HeeksObj* HGear::MakeSketch()const
{
	sketch_for_gear = new CSketch();
	SetSegmentsVariables(NULL);

	for(int i = 0; i<m_num_teeth; i++)
	{
		tooth(i, true, false);
	}

#ifdef GEAR_SKETCH_BIARCS
	CSketch *sketch_with_biarcs = sketch_for_gear->SplineToBiarcs(0.01);
	delete sketch_for_gear;
	sketch_for_gear = NULL;
	return sketch_with_biarcs;
#else
	CSketch* sketch = sketch_for_gear;
	sketch_for_gear = NULL;
	return sketch;
#endif
}

void HGear::GetOneToothSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{
	SetSegmentsVariables(callbackfunc);
	tooth(0, want_start_point, true);
}

static void glVertexFunction(const double *p){glVertex3d(p[0], p[1], p[2]);}

void HGear::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(HeeksColor(0, 0, 0));
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}

	height_for_point = 0.0;
	glBegin(GL_LINE_STRIP);
	GetSegments(glVertexFunction, wxGetApp().GetPixelScale());
	glEnd();

	if(fabs(m_depth) > 0.000000000001)
	{
		height_for_point = m_depth;
		glBegin(GL_LINE_STRIP);
		GetSegments(glVertexFunction, wxGetApp().GetPixelScale());
		glEnd();
	}

	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
}

HeeksObj *HGear::MakeACopy(void)const{
		HGear *new_object = new HGear(*this);
		return new_object;
}

void HGear::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	m_pos.Transform(mat);
}

void HGear::GetBox(CBox &box){
	double acting_radius = (double)(m_module * m_num_teeth)/2;
	double outside_radius = acting_radius + (m_addendum_multiplier*m_module + m_addendum_offset);
	gp_Trsf mat = make_matrix(m_pos.Location(), m_pos.XDirection(), m_pos.YDirection());
	gp_Pnt p[4];
	p[0] = gp_Pnt(m_pos.Location().XYZ() + gp_XYZ(outside_radius, outside_radius, 0.0));
	p[1] = gp_Pnt(m_pos.Location().XYZ() + gp_XYZ(-outside_radius, outside_radius, 0.0));
	p[2] = gp_Pnt(m_pos.Location().XYZ() + gp_XYZ(-outside_radius, -outside_radius, 0.0));
	p[3] = gp_Pnt(m_pos.Location().XYZ() + gp_XYZ(outside_radius, -outside_radius, 0.0));
	for(int i = 0; i<4; i++)
	{
		p[i].Transform(mat);
		box.Insert(p[i].X(), p[i].Y(), p[i].Z());
	}
}

void HGear::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	gp_Pnt o = m_pos.Location();
	double acting_radius = (double)(m_module * m_num_teeth)/2;
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * acting_radius);
	gp_Pnt py(o.XYZ() + m_pos.YDirection().XYZ() * acting_radius);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * acting_radius);
	gp_Pnt pxyz(o.XYZ() + m_pos.XDirection().XYZ() * acting_radius  + m_pos.YDirection().XYZ() * acting_radius + z_dir.XYZ() * acting_radius);
	list->push_back(GripData(GripperTypeTranslate,o.X(),o.Y(),o.Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,px.X(),px.Y(),px.Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,py.X(),py.Y(),py.Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,pz.X(),pz.Y(),pz.Z(),NULL));
	list->push_back(GripData(GripperTypeScale,pxyz.X(),pxyz.Y(),pxyz.Z(),NULL));
}

static void on_set_num_teeth(int value, HeeksObj* object){
	((HGear*)object)->m_num_teeth = value;
}

static void on_set_module(double value, HeeksObj* object){
	((HGear*)object)->m_module = value;
}

static void on_set_pressure_angle(double value, HeeksObj* object){
	((HGear*)object)->m_pressure_angle = value * M_PI/180;
}

static void on_set_tip_relief(double value, HeeksObj* object){
	((HGear*)object)->m_tip_relief = value;
}

static void on_set_depth(double value, HeeksObj* object){
	((HGear*)object)->m_depth = value;
}

static void on_set_cone_half_angle(double value, HeeksObj* object){
	((HGear*)object)->m_cone_half_angle = value * M_PI/180;
}

static void on_set_angle(double value, HeeksObj* object){
	((HGear*)object)->m_angle = value;
}
void HGear::GetProperties(std::list<Property *> *list){
	list->push_back(new PropertyInt(_("num teeth"), m_num_teeth, this, on_set_num_teeth));
	list->push_back(new PropertyDouble(_("module"), m_module, this, on_set_module));
	list->push_back(new PropertyDouble(_("pressure angle"), m_pressure_angle * 180/M_PI, this, on_set_pressure_angle));
	list->push_back(new PropertyDouble(_("tip relief"), m_tip_relief, this, on_set_tip_relief));
	list->push_back(new PropertyDouble(_("depth"), m_depth, this, on_set_depth));
	list->push_back(new PropertyDouble(_("cone half angle"), m_cone_half_angle * 180/M_PI, this, on_set_cone_half_angle));
	list->push_back(new PropertyDouble(_("drawn angle"), m_angle, this, on_set_angle));

	HeeksObj::GetProperties(list);
}

bool HGear::GetScaleAboutMatrix(double *m)
{
	gp_Trsf mat = make_matrix(m_pos.Location(), m_pos.XDirection(), m_pos.YDirection());
	extract(mat, m);
	return true;
}

static HGear* object_for_Tool = NULL;
static bool lines_started = false;
static gp_Pnt prev_point = gp_Pnt(0.0, 0.0, 0.0);
static CSketch* sketch_for_make = NULL;

static void lineAddFunction(const double *p)
{
	gp_Pnt pnt = make_point(p);

	if(lines_started)
	{
		HLine* new_object = new HLine(prev_point, pnt, &wxGetApp().current_color);
		sketch_for_make->Add(new_object, NULL);
	}

	lines_started = true;
	prev_point = pnt;
}

class GearMakeSketches: public Tool
{
public:
	void Run(){
		height_for_point = 0.0;
		lines_started = false;

		void(*callbackfunc)(const double *p) = NULL;
		callbackfunc = lineAddFunction;

		if(oneTooth())
		{
			object_for_Tool->GetOneToothSegments(callbackfunc, wxGetApp().GetPixelScale());
			wxGetApp().Add(sketch_for_make, NULL);
		}
		else
		{
			wxGetApp().Add(object_for_Tool->MakeSketch(), NULL);
		}

		if(fabs(object_for_Tool->m_depth) > 0.000000000001)
		{
			height_for_point = object_for_Tool->m_depth;
			lines_started = false;
			sketch_for_make = new CSketch();
			if(oneTooth())
			{
				object_for_Tool->GetOneToothSegments(callbackfunc, wxGetApp().GetPixelScale());
		wxGetApp().Add(sketch_for_make, NULL);
			}
			else
			{
				wxGetApp().Add(object_for_Tool->MakeSketch(), NULL);
			}
		}
	}
	const wxChar* GetTitle(){return _("Make Sketches");}
	//wxString BitmapPath(){return _T("trsfw");}

	virtual bool oneTooth(){return false;}
};

class GearMakeOneToothSketches: public GearMakeSketches
{
public:
	const wxChar* GetTitle(){return _("Make One Tooth Sketches");}
	bool oneTooth(){return true;}
};

static GearMakeSketches make_sketches;
static GearMakeOneToothSketches make_one_tooth_sketches;

void HGear::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	object_for_Tool = this;
	t_list->push_back(&make_sketches);
	t_list->push_back(&make_one_tooth_sketches);
}

void HGear::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Gear" );
	root->LinkEndChild( element );

	element->SetAttribute("num_teeth", m_num_teeth);
	element->SetDoubleAttribute("module", m_module);
	element->SetDoubleAttribute("addendum_offset", m_addendum_offset);
	element->SetDoubleAttribute("addendum_multiplier", m_addendum_multiplier);
	element->SetDoubleAttribute("dedendum_multiplier", m_dedendum_multiplier);
	element->SetDoubleAttribute("pressure_angle", m_pressure_angle);
	element->SetDoubleAttribute("tip_relief", m_tip_relief);
	element->SetDoubleAttribute("depth", m_depth);
	element->SetDoubleAttribute("cone_half_angle", m_cone_half_angle);
	element->SetDoubleAttribute("drawn_angle", m_angle);

	const gp_Pnt& l = m_pos.Location();
	element->SetDoubleAttribute("lx", l.X());
	element->SetDoubleAttribute("ly", l.Y());
	element->SetDoubleAttribute("lz", l.Z());

	const gp_Dir& d = m_pos.Direction();
	element->SetDoubleAttribute("dx", d.X());
	element->SetDoubleAttribute("dy", d.Y());
	element->SetDoubleAttribute("dz", d.Z());

	const gp_Dir& x = m_pos.XDirection();
	element->SetDoubleAttribute("xx", x.X());
	element->SetDoubleAttribute("xy", x.Y());
	element->SetDoubleAttribute("xz", x.Z());

	WriteBaseXML(element);
}

// static member function
HeeksObj* HGear::ReadFromXMLElement(TiXmlElement* element)
{
	HGear* new_object = new HGear();

	element->Attribute("num_teeth", &new_object->m_num_teeth);
	element->Attribute("module", &new_object->m_module);
	element->Attribute("addendum_offset", &new_object->m_addendum_offset);
	element->Attribute("addendum_multiplier", &new_object->m_addendum_multiplier);
	element->Attribute("dedendum_multiplier", &new_object->m_dedendum_multiplier);
	element->Attribute("pressure_angle", &new_object->m_pressure_angle);
	element->Attribute("tip_relief", &new_object->m_tip_relief);
	element->Attribute("depth", &new_object->m_depth);
	element->Attribute("cone_half_angle", &new_object->m_cone_half_angle);
	element->Attribute("drawn_angle", &new_object->m_angle);

	double l[3] = {0.0, 0.0, 0.0};
	double d[3] = {0.0, 0.0, 1.0};
	double x[3] = {1.0, 0.0, 0.0};

	element->Attribute("lx", &l[0]);
	element->Attribute("ly", &l[1]);
	element->Attribute("lz", &l[2]);

	element->Attribute("dx", &d[0]);
	element->Attribute("dy", &d[1]);
	element->Attribute("dz", &d[2]);

	element->Attribute("xx", &x[0]);
	element->Attribute("xy", &x[1]);
	element->Attribute("xz", &x[2]);

	new_object->m_pos = gp_Ax2(gp_Pnt(l[0], l[1], l[2]), gp_Dir(d[0], d[1], d[2]), gp_Dir(x[0], x[1], x[2]));

	new_object->ReadBaseXML(element);

	return new_object;
}

double HGear::GetClearanceMM()const
{
	// 12 teeth clearance 0.8
	// 20 teeth clearance 0.55
	// 52 teeth clearance 0.4
	// 100000 teeth clearance 0.1

	return (8.4 / ( 7.2 + m_num_teeth/2.5 )) * m_module;
}
