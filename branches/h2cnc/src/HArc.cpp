// HArc.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HArc.h"
#include "HLine.h"
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

HArc::HArc(const HArc &line):EndedObject(&line.color){
#ifndef MULTIPLE_OWNERS
	C = new HPoint(gp_Pnt(),&line.color);
#endif
	operator=(line);
}

HArc::HArc(const gp_Pnt &a, const gp_Pnt &b, const gp_Circ &c, const HeeksColor* col):EndedObject(col),color(*col){ //:color(*col), EndedObject(col){
#ifdef MULTIPLE_OWNERS
	A->m_p = a;
	B->m_p = b;
	C = new HPoint(c.Location(),col);
	C->SetSkipForUndo(true);
	Add(C,NULL);
#else
	A->m_p = a;
	B->m_p = b;
	C = new HPoint(c.Location(),col);
#endif
	C->m_draw_unselected = false;
	m_axis = c.Axis();
	m_radius = c.Radius();
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

	if(arc->C->m_p.Distance(C->m_p) > wxGetApp().m_geom_tol || arc->m_radius != m_radius)
		return true;

	return EndedObject::IsDifferent(other);
}

const HArc& HArc::operator=(const HArc &b){
	EndedObject::operator=(b);
	m_radius = b.m_radius;
	m_axis = b.m_axis;
	color = b.color;
#ifdef MULTIPLE_OWNERS
	std::list<HeeksObj*>::iterator it = m_objects.begin();
	it++;it++;
	C = (HPoint*)*it;
#else
	*C = *b.C;
#endif
	C->SetSkipForUndo(true);
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
#ifdef MULTIPLE_OWNERS
	std::list<HeeksObj*>::iterator it = m_objects.begin();
	it++;it++;
	C = (HPoint*)*it;
#endif
}

HArc* arc_for_tool = NULL;

class ClickArcCentre: public Tool
{
public:
	HArc *pArc;

public:
	void Run()
	{
		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(pArc->C->m_p, DigitizeInputType);
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
		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(pArc->A->m_p, DigitizeInputType);
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
		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(pArc->B->m_p, DigitizeInputType);
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
	if(A->m_p.IsEqual(B->m_p, wxGetApp().m_geom_tol)){
		return;
	}

	gp_Ax2 axis(C->m_p,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C->m_p;

	double ax = gp_Vec(A->m_p.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A->m_p.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B->m_p.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B->m_p.XYZ() - centre.XYZ()) * y_axis;

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
		wxGetApp().glColorEnsuringContrast(color);
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
	wxGetApp().PlotSetColor(color);
	double s[3], e[3], c[3];
	extract(A->m_p, s);
	extract(B->m_p, e);
	extract(C->m_p, c);
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
	C->m_p.Transform(mat);
	m_radius = C->m_p.Distance(A->m_p);
}

void HArc::GetBox(CBox &box){
	box.Insert(A->m_p.X(), A->m_p.Y(), A->m_p.Z());
	box.Insert(B->m_p.X(), B->m_p.Y(), B->m_p.Z());

	if(IsIncluded(gp_Pnt(0,m_radius,0)))
		box.Insert(C->m_p.X(),C->m_p.Y()+m_radius,C->m_p.Z());
	if(IsIncluded(gp_Pnt(0,-m_radius,0)))
		box.Insert(C->m_p.X(),C->m_p.Y()-m_radius,C->m_p.Z());
	if(IsIncluded(gp_Pnt(m_radius,0,0)))
		box.Insert(C->m_p.X()+m_radius,C->m_p.Y(),C->m_p.Z());
	if(IsIncluded(gp_Pnt(-m_radius,0,0)))
		box.Insert(C->m_p.X()-m_radius,C->m_p.Y(),C->m_p.Z());

}

bool HArc::IsIncluded(gp_Pnt pnt)
{
	gp_Ax2 axis(C->m_p,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C->m_p;

	double ax = gp_Vec(A->m_p.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A->m_p.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B->m_p.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B->m_p.XYZ() - centre.XYZ()) * y_axis;

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
	list->push_back(GripData(GripperTypeStretch,C->m_p.X(),C->m_p.Y(),C->m_p.Z(),C));
}

static void on_set_start(const double *vt, HeeksObj* object){
	((HArc*)object)->A->m_p = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_end(const double *vt, HeeksObj* object){
	((HArc*)object)->B->m_p = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_centre(const double *vt, HeeksObj* object){
	((HArc*)object)->C->m_p = make_point(vt);
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
	extract(A->m_p, a);
	extract(B->m_p, b);
	extract(C->m_p, c);
	extract(m_axis.Direction(), ax);
	list->push_back(new PropertyVertex(_("start"), a, this, on_set_start));
	list->push_back(new PropertyVertex(_("end"), b, this, on_set_end));
	list->push_back(new PropertyVertex(_("centre"), c, this, on_set_centre));
	list->push_back(new PropertyVector(_("axis"), ax, this, on_set_axis));
	double length = A->m_p.Distance(B->m_p);
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
	return gp_Circ(gp_Ax2(C->m_p,m_axis.Direction()),A->m_p.Distance(C->m_p));
}

void HArc::SetCircle(gp_Circ c)
{
	m_radius = c.Radius();
	C->m_p = c.Location();
	m_axis = c.Axis();
}

bool HArc::Intersects(const gp_Pnt &pnt)const
{
	if(!intersect(pnt, GetCircle()))return false;

	if(pnt.IsEqual(A->m_p, wxGetApp().m_geom_tol)){
		return true;
	}

	if(pnt.IsEqual(B->m_p, wxGetApp().m_geom_tol)){
		return true;
	}

	if(A->m_p.IsEqual(B->m_p, wxGetApp().m_geom_tol)){
		return false; // no size arc!
	}

	gp_Ax2 axis(C->m_p,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C->m_p;

	double ax = gp_Vec(A->m_p.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A->m_p.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B->m_p.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B->m_p.XYZ() - centre.XYZ()) * y_axis;
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

#ifdef MULTIPLE_OWNERS
void HArc::LoadFromDoubles()
{
	EndedObject::LoadFromDoubles();
	C->LoadFromDoubles();
	m_radius = C->m_p.Distance(A->m_p);
}

void HArc::LoadToDoubles()
{
	EndedObject::LoadToDoubles();
	C->LoadToDoubles();
}
#endif

bool HArc::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this arc is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

bool HArc::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(A->m_p.IsEqual(vp, wxGetApp().m_geom_tol)){
		gp_Vec direction = -(GetSegmentVector(1.0));
		gp_Pnt centre;
		gp_Dir axis;
		gp_Pnt new_A = gp_Pnt(A->m_p.XYZ() + vshift.XYZ());
		if(HArc::TangentialArc(B->m_p, direction, new_A, centre, axis))
		{
			m_axis = gp_Ax1(centre, -axis);
			m_radius = new_A.Distance(centre);
			A->m_p = new_A;
		}
	}
	else if(B->m_p.IsEqual(vp, wxGetApp().m_geom_tol)){
		gp_Vec direction = GetSegmentVector(0.0);
		gp_Pnt centre;
		gp_Dir axis;
		gp_Pnt new_B = gp_Pnt(B->m_p.XYZ() + vshift.XYZ());
		if(HArc::TangentialArc(A->m_p, direction, new_B, centre, axis))
		{
			m_axis = gp_Ax1(centre, axis);
			m_radius = A->m_p.Distance(centre);
			B->m_p = new_B;
		}
	}

	return false;
}

bool HArc::GetCentrePoint(double* pos)
{
	extract(C->m_p, pos);
	return true;
}

gp_Vec HArc::GetSegmentVector(double fraction)const
{
	gp_Pnt centre = C->m_p;
	gp_Pnt p = GetPointAtFraction(fraction);
	gp_Vec vp(centre, p);
	gp_Vec vd = gp_Vec(m_axis.Direction()) ^ vp;
	vd.Normalize();
	return vd;
}

gp_Pnt HArc::GetPointAtFraction(double fraction)const
{
	if(A->m_p.IsEqual(B->m_p, wxGetApp().m_geom_tol)){
		return A->m_p;
	}

	gp_Ax2 axis(C->m_p,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C->m_p;

	double ax = gp_Vec(A->m_p.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A->m_p.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B->m_p.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B->m_p.XYZ() - centre.XYZ()) * y_axis;

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
	element->SetAttribute("col", color.COLORREF_color());
	gp_Dir D = m_axis.Direction();
	element->SetDoubleAttribute("ax", D.X());
	element->SetDoubleAttribute("ay", D.Y());
	element->SetDoubleAttribute("az", D.Z());
	WriteBaseXML(element);
}

// static member function
HeeksObj* HArc::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt p0(1,0,0), p1(0,1,0), centre(0,0,0);
	double axis[3];
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor((long)(a->IntValue()));}
		else if(name == "sx"){p0.SetX(a->DoubleValue());}
		else if(name == "sy"){p0.SetY(a->DoubleValue());}
		else if(name == "sz"){p0.SetZ(a->DoubleValue());}
		else if(name == "ex"){p1.SetX(a->DoubleValue());}
		else if(name == "ey"){p1.SetY(a->DoubleValue());}
		else if(name == "ez"){p1.SetZ(a->DoubleValue());}
		else if(name == "cx"){centre.SetX(a->DoubleValue());}
		else if(name == "cy"){centre.SetY(a->DoubleValue());}
		else if(name == "cz"){centre.SetZ(a->DoubleValue());}
		else if(name == "ax"){axis[0] = a->DoubleValue();}
		else if(name == "ay"){axis[1] = a->DoubleValue();}
		else if(name == "az"){axis[2] = a->DoubleValue();}
	}

	gp_Circ circle(gp_Ax2(centre, gp_Dir(make_vector(axis))), centre.Distance(p0));

	HArc* new_object = new HArc(p0, p1, circle, &c);
	new_object->ReadBaseXML(pElem);

	if(new_object->GetNumChildren()>3)
	{
		//This is a new style arc, with children points
		new_object->Remove(new_object->A);
		new_object->Remove(new_object->B);
		new_object->Remove(new_object->C);
		delete new_object->A;
		delete new_object->B;
		delete new_object->C;
		new_object->A = (HPoint*)new_object->GetFirstChild();
		new_object->B = (HPoint*)new_object->GetNextChild();
		new_object->C = (HPoint*)new_object->GetNextChild();
		new_object->A->m_draw_unselected = false;
		new_object->B->m_draw_unselected = false;
		new_object->C->m_draw_unselected = false;
		new_object->A->SetSkipForUndo(true);
		new_object->B->SetSkipForUndo(true);
		new_object->C->SetSkipForUndo(true);
		new_object->m_radius = new_object->C->m_p.Distance(new_object->A->m_p);
	}
	return new_object;
}

void HArc::Reverse()
{
	HPoint* temp = A;
	A = B;
	B = temp;
	m_axis.Reverse();
#ifdef MULTIPLE_OWNERS
	m_objects.pop_front();
	m_objects.pop_front();
	m_objects.push_front(B);
	m_objects.push_front(A);
#endif
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
