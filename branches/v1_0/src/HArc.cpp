// HArc.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HArc.h"
#include "HLine.h"
#include "HPoint.h"
#include "HILine.h"
#include "HCircle.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyChoice.h"
#include "../tinyxml/tinyxml.h"
#include "../interface/PropertyVertex.h"
#include "../interface/Tool.h"
#include "Gripper.h"
#include "Sketch.h"
#include "Drawing.h"
#include "DigitizeMode.h"

HArc::HArc(const HArc &line):EndedObject(){
	operator=(line);
}

HArc::HArc(const gp_Pnt &a, const gp_Pnt &b, const gp_Circ &c, const HeeksColor* col):EndedObject(){ 
	A = a;
	B = b;
	C = c.Location();
	m_axis = c.Axis();
	m_radius = c.Radius();
	SetColor(*col);
}

HArc::~HArc(){
}

const wxBitmap &HArc::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/arc.png")));
	return *icon;
}

bool HArc::IsDifferent(HeeksObj* other)
{
	HArc* arc = (HArc*)other;

	if(arc->C.Distance(C) > wxGetApp().m_geom_tol || arc->m_radius != m_radius)
		return true;

	return EndedObject::IsDifferent(other);
}

const HArc& HArc::operator=(const HArc &b){
	EndedObject::operator=(b);
	m_radius = b.m_radius;
	m_axis = b.m_axis;
	C = b.C;
	return *this;
}

HeeksObj* HArc::MakeACopyWithID()
{
	HArc* pnew = (HArc*)EndedObject::MakeACopyWithID();
	return pnew;
}

void HArc::ReloadPointers()
{
	EndedObject::ReloadPointers();
}

HArc* arc_for_tool = NULL;

class ClickArcCentre: public Tool
{
public:
	HArc *pArc;

public:
	void Run()
	{
		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(pArc->C, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wxChar* GetTitle(){return _("Click centre point");}
	wxString BitmapPath(){return _T("click_arc_midpoint");}
};

ClickArcCentre click_arc_centre;

class ClickArcEndOne: public Tool
{
public:
	HArc *pArc;

public:
	void Run()
	{
		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(pArc->A, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wxChar* GetTitle(){return _("Click first end point");}
	wxString BitmapPath(){return _T("click_arc_end_one");}
};

ClickArcEndOne click_arc_first_one;

class ClickArcEndTwo: public Tool
{
public:
	HArc *pArc;

public:
	void Run()
	{
		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(pArc->B, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wxChar* GetTitle(){return _("Click second end point");}
	wxString BitmapPath(){return _T("click_arc_end_two");}
};

ClickArcEndTwo click_arc_first_two;

void HArc::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	arc_for_tool = this;

	Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
	if (pDrawingMode != NULL)
	{
		click_arc_centre.pArc = this;
		t_list->push_back(&click_arc_centre);

		click_arc_first_one.pArc = this;
		t_list->push_back(&click_arc_first_one);

		click_arc_first_two.pArc = this;
		t_list->push_back(&click_arc_first_two);
	}
}



//segments - number of segments per full revolution!
//d_angle - determines the direction and the ammount of the arc to draw
void HArc::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{
	if(A.IsEqual(B, wxGetApp().m_geom_tol)){
		return;
	}

	gp_Ax2 axis(C,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C;

	double ax = gp_Vec(A.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B.XYZ() - centre.XYZ()) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(start_angle > end_angle)end_angle += 6.28318530717958;

	double radius = m_radius;
	double d_angle = end_angle - start_angle;
	int segments = (int)(fabs(pixels_per_mm * radius * d_angle / 6.28318530717958 + 1));
	if(segments<3)segments = 3;

    double theta = d_angle / (double)segments;
	while(theta>1.0){segments*=2;theta = d_angle / (double)segments;}
    double tangetial_factor = tan(theta);
    double radial_factor = 1 - cos(theta);

    double x = radius * cos(start_angle);
    double y = radius * sin(start_angle);

	double pp[3];

   for(int i = 0; i < segments + 1; i++)
    {
		gp_Pnt p = centre.XYZ() + x * x_axis.XYZ() + y * y_axis.XYZ();
		extract(p, pp);
		(*callbackfunc)(pp);

        double tx = -y;
        double ty = x;

        x += tx * tangetial_factor;
        y += ty * tangetial_factor;

        double rx = - x;
        double ry = - y;

        x += rx * radial_factor;
        y += ry * radial_factor;
    }
}

static void glVertexFunction(const double *p){glVertex3d(p[0], p[1], p[2]);}

void HArc::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(*GetColor());
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}

	glBegin(GL_LINE_STRIP);
	GetSegments(glVertexFunction, wxGetApp().GetPixelScale());
	glEnd();

	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
	EndedObject::glCommands(select,marked,no_color);
}

void HArc::Draw(wxDC& dc)
{
	wxGetApp().PlotSetColor(*GetColor());
	double s[3], e[3], c[3];
	extract(A, s);
	extract(B, e);
	extract(C, c);
	wxGetApp().PlotArc(s, e, c);
}

HeeksObj *HArc::MakeACopy(void)const{
		HArc *new_object = new HArc(*this);
		return new_object;
}

void HArc::ModifyByMatrix(const double* m){
	EndedObject::ModifyByMatrix(m);
	gp_Trsf mat = make_matrix(m);
	m_axis.Transform(mat);
	C.Transform(mat);
	m_radius = C.Distance(A);
}

void HArc::GetBox(CBox &box){
	box.Insert(A.X(), A.Y(), A.Z());
	box.Insert(B.X(), B.Y(), B.Z());

	if(IsIncluded(gp_Pnt(0,m_radius,0)))
		box.Insert(C.X(),C.Y()+m_radius,C.Z());
	if(IsIncluded(gp_Pnt(0,-m_radius,0)))
		box.Insert(C.X(),C.Y()-m_radius,C.Z());
	if(IsIncluded(gp_Pnt(m_radius,0,0)))
		box.Insert(C.X()+m_radius,C.Y(),C.Z());
	if(IsIncluded(gp_Pnt(-m_radius,0,0)))
		box.Insert(C.X()-m_radius,C.Y(),C.Z());

}

bool HArc::IsIncluded(gp_Pnt pnt)
{
	gp_Ax2 axis(C,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C;

	double ax = gp_Vec(A.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B.XYZ() - centre.XYZ()) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(start_angle > end_angle)end_angle += 6.28318530717958;

	double pnt_angle = atan2(gp_Vec(pnt.XYZ()) * y_axis, gp_Vec(pnt.XYZ()) * x_axis);
	if(pnt_angle >= start_angle && pnt_angle <= end_angle)
		return true;
	return false;
}

void HArc::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	EndedObject::GetGripperPositions(list,just_for_endof);
	list->push_back(GripData(GripperTypeStretch,C.X(),C.Y(),C.Z(),&C));
}

static void on_set_start(const double *vt, HeeksObj* object){
	((HArc*)object)->A = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_end(const double *vt, HeeksObj* object){
	((HArc*)object)->B = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_centre(const double *vt, HeeksObj* object){
	((HArc*)object)->C = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_axis(const double *vt, HeeksObj* object){
	gp_Ax1 a = ((HArc*)object)->m_axis;
	a.SetDirection(make_vector(vt));
	((HArc*)object)->m_axis = a;
	wxGetApp().Repaint();
}

void HArc::GetProperties(std::list<Property *> *list){
	double a[3], b[3];
	double c[3], ax[3];
	extract(A, a);
	extract(B, b);
	extract(C, c);
	extract(m_axis.Direction(), ax);
	list->push_back(new PropertyVertex(_("start"), a, this, on_set_start));
	list->push_back(new PropertyVertex(_("end"), b, this, on_set_end));
	list->push_back(new PropertyVertex(_("centre"), c, this, on_set_centre));
	list->push_back(new PropertyVector(_("axis"), ax, this, on_set_axis));
	double length = A.Distance(B);
	list->push_back(new PropertyLength(_("length"), length, NULL));
	list->push_back(new PropertyLength(_("radius"), m_radius, this, NULL));

	HeeksObj::GetProperties(list);
}

int HArc::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
	int numi = 0;

	switch(object->GetType())
	{
    case SketchType:
        return( ((CSketch *)object)->Intersects( this, rl ));

	case LineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HLine*)object)->GetLine(), GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(Intersects(pnt) && ((HLine*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HILine*)object)->GetLine(), GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(GetCircle(), ((HArc*)object)->GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(Intersects(pnt) && ((HArc*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case CircleType:
		{
			std::list<gp_Pnt> plist;
			intersect(GetCircle(), ((HCircle*)object)->GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;
	}

	return numi;
}

gp_Circ HArc::GetCircle() const
{
	return gp_Circ(gp_Ax2(C,m_axis.Direction()),A.Distance(C));
}

void HArc::SetCircle(gp_Circ c)
{
	m_radius = c.Radius();
	C = c.Location();
	m_axis = c.Axis();
}

bool HArc::Intersects(const gp_Pnt &pnt)const
{
	if(!intersect(pnt, GetCircle()))return false;

	if(pnt.IsEqual(A, wxGetApp().m_geom_tol)){
		return true;
	}

	if(pnt.IsEqual(B, wxGetApp().m_geom_tol)){
		return true;
	}

	if(A.IsEqual(B, wxGetApp().m_geom_tol)){
		return false; // no size arc!
	}

	gp_Ax2 axis(C,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C;

	double ax = gp_Vec(A.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B.XYZ() - centre.XYZ()) * y_axis;
	double px = gp_Vec(pnt.XYZ() - centre.XYZ()) * x_axis;
	double py = gp_Vec(pnt.XYZ() - centre.XYZ()) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);
	double pnt_angle = atan2(py, px);

	// force the angle to be greater than start angle
	if(start_angle > end_angle)end_angle += 6.28318530717958;
	while(pnt_angle < start_angle)pnt_angle += 6.28318530717958;

	// point lies on the arc, if the angle is less than the end angle
	return pnt_angle < end_angle;
}

bool HArc::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< gp_Pnt > rl;
	ClosestPointsLineAndCircle(ray, GetCircle(), rl);
	if(rl.size()>0)
	{
		gp_Pnt p = rl.front();
		if(Intersects(p))
		{
			extract(p, point);
			return true;
		}
	}

	return false;
}

bool HArc::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this arc is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

bool HArc::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(A.IsEqual(vp, wxGetApp().m_geom_tol)){
		gp_Vec direction = -(GetSegmentVector(1.0));
		gp_Pnt centre;
		gp_Dir axis;
		gp_Pnt new_A = gp_Pnt(A.XYZ() + vshift.XYZ());
		if(HArc::TangentialArc(B, direction, new_A, centre, axis))
		{
			m_axis = gp_Ax1(centre, -axis);
			m_radius = new_A.Distance(centre);
			A = new_A;
		}
	}
	else if(B.IsEqual(vp, wxGetApp().m_geom_tol)){
		gp_Vec direction = GetSegmentVector(0.0);
		gp_Pnt centre;
		gp_Dir axis;
		gp_Pnt new_B = gp_Pnt(B.XYZ() + vshift.XYZ());
		if(HArc::TangentialArc(A, direction, new_B, centre, axis))
		{
			m_axis = gp_Ax1(centre, axis);
			m_radius = A.Distance(centre);
			B = new_B;
		}
	}

	return false;
}

bool HArc::GetCentrePoint(double* pos)
{
	extract(C, pos);
	return true;
}

gp_Vec HArc::GetSegmentVector(double fraction)const
{
	gp_Pnt centre = C;
	gp_Pnt p = GetPointAtFraction(fraction);
	gp_Vec vp(centre, p);
	gp_Vec vd = gp_Vec(m_axis.Direction()) ^ vp;
	vd.Normalize();
	return vd;
}

gp_Pnt HArc::GetPointAtFraction(double fraction)const
{
	if(A.IsEqual(B, wxGetApp().m_geom_tol)){
		return A;
	}

	gp_Ax2 axis(C,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C;

	double ax = gp_Vec(A.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B.XYZ() - centre.XYZ()) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(start_angle > end_angle)end_angle += 6.28318530717958;

	double radius = m_radius;
	double d_angle = end_angle - start_angle;
	double angle = start_angle + d_angle * fraction;
    double x = radius * cos(angle);
    double y = radius * sin(angle);

	return centre.XYZ() + x * x_axis.XYZ() + y * y_axis.XYZ();
}

//static
bool HArc::TangentialArc(const gp_Pnt &p0, const gp_Vec &v0, const gp_Pnt &p1, gp_Pnt &centre, gp_Dir &axis)
{
	// returns false if a straight line is needed
	// else returns true and sets centre and axis
	if(p0.Distance(p1) > 0.0000000001 && v0.Magnitude() > 0.0000000001){
		gp_Vec v1(p0, p1);
		gp_Pnt halfway(p0.XYZ() + v1.XYZ() * 0.5);
		gp_Pln pl1(halfway, v1);
		gp_Pln pl2(p0, v0);
		gp_Lin plane_line;
		if(intersect(pl1, pl2, plane_line))
		{
			gp_Lin l1(halfway, v1);
			gp_Pnt unused_p2;
			ClosestPointsOnLines(plane_line, l1, centre, unused_p2);
			axis = -(plane_line.Direction());
			return true;
		}
	}

	return false; // you'll have to do a line instead
}

void HArc::WriteXML(TiXmlNode *root)
{
	TiXmlElement *element = new TiXmlElement( "Arc" );
	root->LinkEndChild( element );
	gp_Dir D = m_axis.Direction();
	element->SetDoubleAttribute("cx", C.X());
	element->SetDoubleAttribute("cy", C.Y());
	element->SetDoubleAttribute("cz", C.Z());
	element->SetDoubleAttribute("ax", D.X());
	element->SetDoubleAttribute("ay", D.Y());
	element->SetDoubleAttribute("az", D.Z());
	WriteBaseXML(element);
}

// static member function
HeeksObj* HArc::ReadFromXMLElement(TiXmlElement* pElem)
{
	double axis[3];
	gp_Pnt centre(0,0,0);
	HeeksColor c;

	// get the attributes
	double x;
	if(pElem->Attribute("ax", &x))axis[0] = x;
	if(pElem->Attribute("ay", &x))axis[1] = x;
	if(pElem->Attribute("az", &x))axis[2] = x;
	if(pElem->Attribute("cx", &x))centre.SetX(x);
	if(pElem->Attribute("cy", &x))centre.SetY(x);
	if(pElem->Attribute("cz", &x))centre.SetZ(x);

	else
	{
		// try the version where the points were children
		int num_points = 0;
		for(TiXmlElement* pElem2 = TiXmlHandle(pElem).FirstChildElement().Element(); pElem2;	pElem2 = pElem2->NextSiblingElement())
		{
			HeeksObj* object = wxGetApp().ReadXMLElement(pElem2);
			if(object->GetType() == PointType)
			{
				num_points++;
				if(num_points == 3)
				{
					centre = ((HPoint*)object)->m_p;
					delete object;
					break;
				}
			}
			delete object;
		}
	}

	HArc* new_object = new HArc(gp_Pnt(), gp_Pnt(), gp_Circ(), &c);
	new_object->ReadBaseXML(pElem);

	gp_Circ circle(gp_Ax2(centre, gp_Dir(make_vector(axis))), centre.Distance(new_object->A));
	new_object->SetCircle(circle);

	return new_object;
}

void HArc::Reverse()
{
	gp_Pnt temp = A;
	A = B;
	B = temp;
	m_axis.Reverse();
}

double HArc::IncludedAngle()const
{
	gp_Vec vs = GetSegmentVector(0.0);
	gp_Vec ve = GetSegmentVector(1.0);

	double inc_ang = vs * ve;
	int dir = (this->m_axis.Direction().Z() > 0) ? 1:-1;
	if(inc_ang > 1. - 1.0e-10) return 0;
	if(inc_ang < -1. + 1.0e-10)
		inc_ang = M_PI;  
	else {									// dot product,   v1 . v2  =  cos ang
		if(inc_ang > 1.0) inc_ang = 1.0;
		inc_ang = acos(inc_ang);									// 0 to M_PI radians

		if(dir * (vs ^ ve).Z() < 0) inc_ang = 2 * M_PI - inc_ang ;		// cp
	}
	return dir * inc_ang;
}
