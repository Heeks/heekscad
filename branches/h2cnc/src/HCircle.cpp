// HCircle.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HCircle.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyChoice.h"
#include "../interface/PropertyVertex.h"
#include "HLine.h"
#include "HILine.h"
#include "HArc.h"
#include "Gripper.h"
#include "DigitizeMode.h"
#include "Drawing.h"

HCircle::HCircle(const HCircle &c){
	operator=(c);
}

HCircle::HCircle(const gp_Circ &c, const HeeksColor* col):color(*col){
	m_axis = c.Axis();
	m_radius = c.Radius();
	C = new HPoint(c.Location(),col);
	C->SetSkipForUndo(true);
	Add(C,NULL);
}

HCircle::~HCircle(){
}

const HCircle& HCircle::operator=(const HCircle &c){
#ifdef MULTIPLE_OWNERS
	ObjList::operator=(c);
#else
	HeeksObj::operator=(c);
#endif
	m_axis = c.m_axis;
	m_radius = c.m_radius;
	color = c.color;
	C = new HPoint(c.C->m_p,&color);
	C->SetSkipForUndo(true);
	Add(C,NULL);
	return *this;
}

const wxBitmap &HCircle::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/circle.png")));
	return *icon;
}

bool HCircle::IsDifferent(HeeksObj* other)
{
	HCircle* cir = (HCircle*)other;
	if(cir->C->m_p.Distance(C->m_p) > wxGetApp().m_geom_tol)
		return true;

	if(!IsEqual(cir->m_axis,m_axis))
		return true;

	if(cir->m_radius != m_radius)
		return true;

	return HeeksObj::IsDifferent(other);
}

//segments - number of segments per full revolution!
void HCircle::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{
	gp_Ax2 axis(C->m_p,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_Pnt centre = C->m_p;

	double radius = m_radius;
	int segments = (int)(fabs(pixels_per_mm * radius + 1));

    double theta = 6.28318530717958 / (double)segments;
    double tangetial_factor = tan(theta);
    double radial_factor = 1 - cos(theta);

    double x = radius;
    double y = 0.0;

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

void HCircle::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(color);
		if (wxGetApp().m_allow_opengl_stippling)
		{
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(3, 0xaaaa);
		}
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

	if(!no_color)
	{
		if (wxGetApp().m_allow_opengl_stippling)
		{
			glDisable(GL_LINE_STIPPLE);
		}
	}
}

HeeksObj *HCircle::MakeACopy(void)const{
		HCircle *new_object = new HCircle(*this);
		return new_object;
}

void HCircle::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	m_axis.Transform(mat);
	C->m_p.Transform(mat);
	m_radius *= mat.ScaleFactor();
}

void HCircle::GetBox(CBox &box){
	gp_Ax2 axis(C->m_p,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Dir y_axis = axis.YDirection();
	gp_XYZ c = C->m_p.XYZ();
	double r = m_radius;
	gp_XYZ x = x_axis.XYZ() * r;
	gp_XYZ y = y_axis.XYZ() * r;

	gp_Pnt p[4];
	p[0] = gp_Pnt(c - x - y);
	p[1] = gp_Pnt(c + x - y);
	p[2] = gp_Pnt(c + x + y);
	p[3] = gp_Pnt(c - x + y);

	for(int i = 0; i<4; i++)
	{
		double pp[3];
		extract(p[i], pp);
		box.Insert(pp);
	}
}

void HCircle::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	if(!just_for_endof)
	{
		gp_Ax2 axis(C->m_p,m_axis.Direction());
		gp_Dir x_axis = axis.XDirection();
		gp_XYZ c = C->m_p.XYZ();
		double r = m_radius;
		gp_Pnt s(c + x_axis.XYZ() * r);

		list->push_back(GripData(GripperTypeStretch,c.X(),c.Y(),c.Z(),C));
		list->push_back(GripData(GripperTypeStretch,s.X(),s.Y(),s.Z(),&m_radius));
	}
}

static void on_set_centre(const double *vt, HeeksObj* object){
	((HCircle*)object)->C->m_p = make_point(vt);
	wxGetApp().Repaint();
}

static void on_set_axis(const double *vt, HeeksObj* object){
	((HCircle*)object)->m_axis.SetDirection(make_vector(vt).XYZ());
	wxGetApp().Repaint();
}

static void on_set_radius(double value, HeeksObj* object){
	((HCircle*)object)->m_radius = value;
	wxGetApp().Repaint();
}

void HCircle::GetProperties(std::list<Property *> *list){
	double c[3], a[3];
	extract(C->m_p, c);
	extract(m_axis.Direction(), a);
	list->push_back(new PropertyVertex(_("centre"), c, this, on_set_centre));
	list->push_back(new PropertyVector(_("axis"), a, this, on_set_axis));
	list->push_back(new PropertyLength(_("radius"), m_radius, this, on_set_radius));

	HeeksObj::GetProperties(list);
}

bool HCircle::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< gp_Pnt > rl;
	ClosestPointsLineAndCircle(ray, GetCircle(), rl);
	if(rl.size()>0)
	{
		extract(rl.front(), point);
		return true;
	}

	return false;
}

bool HCircle::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this circle is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

bool HCircle::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	gp_Ax2 axis(C->m_p,m_axis.Direction());
	gp_Dir x_axis = axis.XDirection();
	gp_Pnt c = C->m_p.XYZ();
	double r = m_radius;
	gp_Pnt s(c.XYZ() + x_axis.XYZ() * r);

	if(data == C){
		C->m_p = vp.XYZ() + vshift.XYZ();
	}
	else if(data == &m_radius)
	{
		s = gp_Pnt(vp.XYZ() + vshift.XYZ());
		m_radius = c.Distance(s);
	}
	return false;
}

bool HCircle::GetCentrePoint(double* pos)
{
	extract(C->m_p, pos);
	return true;
}

void HCircle::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Circle" );
	root->LinkEndChild( element );
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("r", m_radius);
	gp_Dir D = m_axis.Direction();
	element->SetDoubleAttribute("cx", C->m_p.X());
	element->SetDoubleAttribute("cy", C->m_p.Y());
	element->SetDoubleAttribute("cz", C->m_p.Z());
	element->SetDoubleAttribute("ax", D.X());
	element->SetDoubleAttribute("ay", D.Y());
	element->SetDoubleAttribute("az", D.Z());
	WriteBaseXML(element);
}

// static member function
HeeksObj* HCircle::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt centre;
	double axis[3];
	double r = 0.0;
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor((long)(a->IntValue()));}
		else if(name == "r"){r = a->DoubleValue();}
		else if(name == "cx"){centre.SetX(a->DoubleValue());}
		else if(name == "cy"){centre.SetY(a->DoubleValue());}
		else if(name == "cz"){centre.SetZ(a->DoubleValue());}
		else if(name == "ax"){axis[0] = a->DoubleValue();}
		else if(name == "ay"){axis[1] = a->DoubleValue();}
		else if(name == "az"){axis[2] = a->DoubleValue();}
	}

	gp_Circ circle(gp_Ax2(centre, gp_Dir(make_vector(axis))), r);

	HCircle* new_object = new HCircle(circle, &c);
	new_object->ReadBaseXML(pElem);

	return new_object;
}

int HCircle::Intersects(const HeeksObj *object, std::list< double > *rl)const
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
				if(((HLine*)object)->Intersects(pnt))
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
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;

	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(GetCircle(), ((HArc*)object)->GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(((HArc*)object)->Intersects(pnt))
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
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;
	}

	return numi;
}

//static
bool HCircle::GetLineTangentPoints(const gp_Circ& c1, const gp_Circ& c2, const gp_Pnt& a, const gp_Pnt& b, gp_Pnt& p1, gp_Pnt& p2)
{
	// find the tangent points for a line between two circles

	// they need to be flat ( compared with each other )
	if(fabs(fabs(c1.Axis().Direction() * c2.Axis().Direction()) - 1.0) > 0.00000001)return false;
	if(fabs( gp_Vec(c1.Location().XYZ()) * c1.Axis().Direction() - gp_Vec(c2.Location().XYZ()) * c1.Axis().Direction()) > 0.00000001)return false;

	// not concentric
	if(c1.Location().IsEqual(c2.Location(), wxGetApp().m_geom_tol))return false;

	// find left and right
	gp_Vec join(c1.Location(), c2.Location());
	gp_Vec forward = join.Normalized();
	gp_Vec left1 = c1.Axis().Direction() ^ join;
	gp_Vec left2 = c2.Axis().Direction() ^ join;
	if(left1 * left2 < 0)left2 = -left2;
	gp_Vec c1a(c1.Location(), a);
	gp_Vec c2b(c2.Location(), b);

	bool is_left1 = c1a * left1 > 0;
	bool is_left2 = c2b * left2 > 0;

	double r1 = c1.Radius();
	double r2 = c2.Radius();

	double H = join.Magnitude();
	double O = (is_left1 == is_left2) ? fabs(r1 - r2) : (r1 + r2);
	double A = sqrt(H*H - O*O);
	double sinang = O/H;
	double cosang = A/H;

	double f1 = 1.0;
	double l1 = is_left1 ? 1.0:-1.0;
	double f2 = 1.0;
	double l2 = is_left2 ? 1.0:-1.0;

	if(is_left1 == is_left2){
		if(r1 < r2)
		{
			f1 = -1.0;
			f2 = -1.0;
		}
	}
	else
	{
		f1 = 1.0;
		f2 = -1.0;
	}

	p1 = c1.Location().XYZ() + forward.XYZ() * (f1 * r1 * sinang) + left1.XYZ() * (l1 * r1 * cosang);
	p2 = c2.Location().XYZ() + forward.XYZ() * (f2 * r2 * sinang) + left2.XYZ() * (l2 * r2 * cosang);

	return true;
}

// static
bool HCircle::GetLineTangentPoint(const gp_Circ& c, const gp_Pnt& a, const gp_Pnt& b, gp_Pnt& p)
{
	// find the tangent point for a line from near point "a" on the circle to point "b"

	// find left and right
	gp_Vec join(c.Location(), b);
	gp_Vec forward = join.Normalized();
	gp_Vec left = c.Axis().Direction() ^ join;
	gp_Vec ca(c.Location(), a);

	bool is_left = ca * left > 0;
	double r = c.Radius();

	double H = join.Magnitude();
	double O = r;
	double A = sqrt(H*H - O*O);
	double sinang = O/H;
	double cosang = A/H;

	double l = is_left ? 1.0:-1.0;
	p = c.Location().XYZ() + forward.XYZ() * (r * sinang) + left.XYZ() * (l * r * cosang);

	return true;
}

// static
bool HCircle::GetArcTangentPoints(const gp_Circ& c, const gp_Lin &line, const gp_Pnt& p, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis)
{
	// find the arc that fits tangentially from the line to the circle
	// returns p1 - on the line, p2 - on the circle, centre - the centre point of the arc

	// they need to be flat ( compared with each other )
	if(fabs(gp_Vec(c.Axis().Direction()) * gp_Vec(line.Direction())) > 0.00000001)return false;
	if(fabs( gp_Vec(c.Location().XYZ()) * c.Axis().Direction() - gp_Vec(line.Location().XYZ()) * c.Axis().Direction()) > 0.00000001)return false;

	// make a concentric circle
	bool p_infront_of_centre = gp_Vec(p.XYZ()) * gp_Vec(line.Direction()) > gp_Vec(c.Location().XYZ()) * gp_Vec(line.Direction());
	double R = p_infront_of_centre ? (c.Radius() - radius) : (c.Radius() + radius);
	if(R < 0.000000000000001)return false;
	gp_Circ concentric_circle(gp_Ax2(c.Axis().Location(), c.Axis().Direction()), R);

	// make an offset line
	gp_Vec left = c.Axis().Direction() ^ line.Direction();
	bool p_on_left_of_line = gp_Vec(p.XYZ()) * left > gp_Vec(line.Location().XYZ()) * left;
	gp_Lin offset_line(gp_Pnt(line.Location().XYZ() + left.XYZ() * (p_on_left_of_line ? radius:-radius)), line.Direction());

	// find intersection between concentric circle and offset line
	{
		std::list<gp_Pnt> rl;
		intersect(offset_line, concentric_circle, rl);
		double best_dp = 0.0;
		gp_Pnt* best_pnt = NULL;
		for(std::list<gp_Pnt>::iterator It = rl.begin(); It != rl.end(); It++)
		{
			gp_Pnt& pnt = *It;
			double dp = gp_Vec(pnt.XYZ()) * gp_Vec(line.Direction());
			if(best_pnt == NULL || (p_infront_of_centre && dp > best_dp) || (!p_infront_of_centre && dp < best_dp))
			{
				best_pnt = &pnt;
				best_dp = dp;
			}
		}

		if(best_pnt == NULL)return false;

		// use as centre of arc
		centre = *best_pnt;
	}

	// make a circle at this point
	gp_Circ circle_for_arc(gp_Ax2(centre, c.Axis().Direction()), radius);

	// intersect with original line
	{
		std::list<gp_Pnt> rl;
		intersect(line, circle_for_arc, rl);
		if(rl.size() == 0)return false;

		// use this point for p1
		p1 = rl.front();
	}

	// intersect with original circle
	{
		std::list<gp_Pnt> rl;
		intersect(c, circle_for_arc, rl);
		if(rl.size() == 0)return false;

		// use this point for p2
		p2 = rl.front();
	}

	axis = c.Axis().Direction();

	return true;
}

class two_circles{
public:
	gp_Circ m_c1;
	gp_Circ m_c2;

	two_circles(const gp_Circ& c1, const gp_Circ& c2)
	{
		m_c1 = c1;
		m_c2 = c2;
	}
};

class two_points{
public:
	gp_Pnt m_p1;
	gp_Pnt m_p2;
	gp_Pnt m_c;

	two_points(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& c)
	{
		m_p1 = p1;
		m_p2 = p2;
		m_c = c;
	}
};

static const two_points* find_best_points_pair(const std::list<two_points>& point_pairs, const gp_Pnt& a, const gp_Pnt& b)
{
	const two_points* best_pair = NULL;
	double best_pair_error = 0.0;
	for(std::list<two_points>::const_iterator It = point_pairs.begin(); It != point_pairs.end(); It++)
	{
		const two_points& point_pair = *It;
		double d1 = a.Distance(point_pair.m_p1);
		double d2 = b.Distance(point_pair.m_p2);
		double pair_error = d1 + d2;
		if(best_pair == NULL || pair_error < best_pair_error)
		{
			best_pair = &point_pair;
			best_pair_error = pair_error;
		}
	}

	return best_pair;
}

// static
bool HCircle::GetArcTangentPoints(const gp_Circ& c1, const gp_Circ &c2, const gp_Pnt& a, const gp_Pnt& b, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis)
{
	// find the arc that fits tangentially from the circle to the circle
	// returns p1 - on c1, p2 - on c2, centre - the centre point of the arc

	// they need to be flat ( compared with each other )
	if(fabs(fabs(c1.Axis().Direction() * c2.Axis().Direction()) - 1.0) > 0.00000001)return false;
	if(fabs( gp_Vec(c1.Location().XYZ()) * c1.Axis().Direction() - gp_Vec(c2.Location().XYZ()) * c1.Axis().Direction()) > 0.00000001)return false;

	// not concentric
	if(c1.Location().IsEqual(c2.Location(), wxGetApp().m_geom_tol))return false;

	// find left and right
	gp_Vec join(c1.Location(), c2.Location());
	gp_Vec forward = join.Normalized();
	gp_Vec left1 = c1.Axis().Direction() ^ join;
	gp_Vec left2 = c2.Axis().Direction() ^ join;
	if(left1 * left2 < 0)left2 = -left2;
	gp_Vec c1a(c1.Location(), a);
	gp_Vec c2b(c2.Location(), b);

	double r1 = c1.Radius();
	double r2 = c2.Radius();

	std::list<gp_Circ> c1_list, c2_list;
	c1_list.push_back(gp_Circ(gp_Ax2(c1.Location(), c1.Axis().Direction()), r1 + radius));
	if(radius > r1)c1_list.push_back(gp_Circ(gp_Ax2(c1.Location(), c1.Axis().Direction()), radius - r1));
	c2_list.push_back(gp_Circ(gp_Ax2(c2.Location(), c2.Axis().Direction()), r2 + radius));
	if(radius > r2)c2_list.push_back(gp_Circ(gp_Ax2(c2.Location(), c2.Axis().Direction()), radius - r2));

	std::list<two_circles> combinations;
	for(std::list<gp_Circ>::iterator It1 = c1_list.begin(); It1 != c1_list.end(); It1++)
	{
		for(std::list<gp_Circ>::iterator It2 = c2_list.begin(); It2 != c2_list.end(); It2++)
		{
			combinations.push_back(two_circles(*It1, *It2));
		}
	}

	std::list<gp_Pnt> intersections;
	for(std::list<two_circles>::iterator It = combinations.begin(); It != combinations.end(); It++)
	{
		two_circles& circles = *It;
		intersect(circles.m_c1, circles.m_c2, intersections);
	}

	std::list<two_points> point_pairs;
	for(std::list<gp_Pnt>::iterator It = intersections.begin(); It != intersections.end(); It++)
	{
		gp_Pnt& pnt = *It;
		gp_Circ arc_circle(gp_Ax2(pnt, c1.Axis().Direction()), radius);
		std::list<gp_Pnt> rl1, rl2;
		intersect(arc_circle, c1, rl1);
		intersect(arc_circle, c2, rl2);
		if(rl1.size() > 0 && rl2.size() > 0)
		{
			point_pairs.push_back(two_points(rl1.front(), rl2.front(), arc_circle.Location()));
		}
	}

	const two_points* best_pair = find_best_points_pair(point_pairs, a, b);

	if(best_pair)
	{
		p1 = best_pair->m_p1;
		p2 = best_pair->m_p2;
		centre = best_pair->m_c;
		axis = c1.Axis().Direction();
		return true;
	}

	return false;
}

// static
bool HCircle::GetArcTangentPoints(const gp_Lin& l1, const gp_Lin &l2, const gp_Pnt& a, const gp_Pnt& b, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis)
{
	// cross product to find axis of arc
	gp_Vec xp = gp_Vec(l1.Direction()) ^ gp_Vec(l2.Direction());
	if(xp.Magnitude() < 0.00000000000001)return false;
	axis = gp_Dir(xp);

	gp_Vec left1 = axis ^ l1.Direction();
	gp_Vec left2 = axis ^ l2.Direction();

	gp_Lin offset_l1_left(l1.Location().XYZ() + left1.XYZ() * radius, l1.Direction());
	gp_Lin offset_l1_right(l1.Location().XYZ() - left1.XYZ() * radius, l1.Direction());
	gp_Lin offset_l2_left(l2.Location().XYZ() + left2.XYZ() * radius, l2.Direction());
	gp_Lin offset_l2_right(l2.Location().XYZ() - left2.XYZ() * radius, l2.Direction());

	std::list<gp_Circ> circles;

	gp_Pnt pnt;
	if(intersect(offset_l1_left, offset_l2_left, pnt))
		circles.push_back(gp_Circ(gp_Ax2(pnt, axis), radius));
	if(intersect(offset_l1_left, offset_l2_right, pnt))
		circles.push_back(gp_Circ(gp_Ax2(pnt, axis), radius));
	if(intersect(offset_l1_right, offset_l2_left, pnt))
		circles.push_back(gp_Circ(gp_Ax2(pnt, axis), radius));
	if(intersect(offset_l1_right, offset_l2_right, pnt))
		circles.push_back(gp_Circ(gp_Ax2(pnt, axis), radius));

	std::list<two_points> point_pairs;

	for(std::list<gp_Circ>::iterator It = circles.begin(); It != circles.end(); It++)
	{
		gp_Circ& c = *It;
		std::list<gp_Pnt> rl1, rl2;
		intersect(l1, c, rl1);
		intersect(l2, c, rl2);
		if(rl1.size() > 0 && rl2.size() > 0)
		{
			point_pairs.push_back(two_points(rl1.front(), rl2.front(), c.Location()));
		}
	}

	const two_points* best_pair = find_best_points_pair(point_pairs, a, b);

	if(best_pair)
	{
		p1 = best_pair->m_p1;
		p2 = best_pair->m_p2;
		centre = best_pair->m_c;
		return true;
	}

	return false;
}

// static
bool HCircle::GetArcTangentPoint(const gp_Lin& l, const gp_Pnt& a, const gp_Pnt& b, const gp_Vec *final_direction, double* radius, gp_Pnt& p, gp_Pnt& centre, gp_Dir& axis)
{
	// find the tangent point on the line l, for an arc from near point "a" on the line to exact given point "b"

	gp_Pnt c = ClosestPointOnLine(l, b);
	if(c.Distance(b) < 0.000000000001)return false;
	gp_Dir sideways(gp_Vec(c, b));

	if(final_direction)
	{
		if((*final_direction) * gp_Vec(sideways.XYZ()) >= 0)// b on correct side compared with final direction
		{
			gp_Lin final_dir_line(b, gp_Dir(final_direction->XYZ()));
			if(!l.Direction().IsEqual(gp_Dir(final_direction->XYZ()), 0.00000001) && !l.Direction().IsEqual(-gp_Dir(final_direction->XYZ()), 0.00000001))
			{
				axis = l.Direction() ^ final_dir_line.Direction();
				gp_Dir perp = axis ^ (*final_direction);
				gp_Lin perp_line(gp_Ax1(b, perp));
				gp_Dir half_angle_dir[2];
				half_angle_dir[0] = gp_Dir(perp.XYZ() + sideways.XYZ());
				half_angle_dir[1] = gp_Dir(sideways.XYZ() - perp.XYZ());


				std::list<gp_Pnt> maybe_p;
				for(int i = 0; i<2; i++)
				{
					gp_Pnt pnt;
					if(intersect(final_dir_line, l, pnt))
					{
						gp_Lin half_angle_line(pnt, half_angle_dir[i]);
						if(intersect(half_angle_line, perp_line, centre))
						{
							double R = b.Distance(centre);
							std::list<gp_Pnt> plist;
							intersect(l, gp_Circ(gp_Ax2(centre, axis), R), plist);
							if(plist.size() == 1)
							{
								maybe_p.push_back(plist.front());
							}
						}
					}
				}

				gp_Pnt* best_pnt = NULL;
				double best_dist = 0.0;

				for(std::list<gp_Pnt>::iterator It = maybe_p.begin(); It != maybe_p.end(); It++)
				{
					gp_Pnt& pnt = *It;
					double dist = a.Distance(pnt);
					if(best_pnt == NULL || dist < best_dist)
					{
						best_pnt = &pnt;
						best_dist = dist;
					}
				}

				if(best_pnt)
				{
					p = *best_pnt;
					return true;
				}
			}
		}
		gp_Vec v0 = -(*final_direction);
		return HArc::TangentialArc(b, v0, a, centre, axis);
	}
	else
	{
		double radius_to_use;
		if(radius)radius_to_use = *radius;
		else
		{
			// fit the minimum radius arc
			radius_to_use = c.Distance(b);
		}
		axis = l.Direction() ^ sideways;
		gp_Circ offset_circle(gp_Ax2(b, axis), radius_to_use);
		gp_Lin offset_line(l.Location().XYZ() + sideways.XYZ() * radius_to_use, l.Direction());
		std::list<gp_Pnt> plist;
		intersect(offset_line, offset_circle, plist);
		gp_Pnt* best_pnt = NULL;
		double best_distance = 0.0;
		for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
		{
			gp_Pnt& pnt = *It;
			double distance = a.Distance(pnt);
			if(best_pnt == NULL || distance < best_distance)
			{
				best_pnt = &pnt;
				best_distance = distance;
			}
		}
		if(best_pnt)
		{
			centre = *best_pnt;
			std::list<gp_Pnt> rl;
			intersect(l, gp_Circ(gp_Ax2(centre, axis), radius_to_use), rl);
			if(rl.size() == 1)
			{
				p = rl.front();
				return true;
			}
		}
	}

	return false;
}

// static
bool HCircle::GetArcTangentPoint(const gp_Circ& c, const gp_Pnt& a, const gp_Pnt& b, const gp_Vec *final_direction, double* radius, gp_Pnt& p, gp_Pnt& centre, gp_Dir& axis)
{
	// find the tangent point on the circle c, for an arc from near point "a" on the circle to exact given point "b"
	if(final_direction)
	{
		// get tangent circles
		gp_Lin final_dir_line(b, gp_Dir(final_direction->XYZ()));
		std::list<gp_Circ> c_list;
		TangentCircles(c, final_dir_line, b, c_list);

		gp_Circ* best_pnt_circle = NULL;
		gp_Pnt best_pnt;
		double best_dist = 0.0;

		for(std::list<gp_Circ>::iterator It = c_list.begin(); It != c_list.end(); It++)
		{
			gp_Circ& circle = *It;
			std::list<gp_Pnt> p_list;
			intersect(circle, c, p_list);
			if(p_list.size() == 1)
			{
				gp_Pnt& p = p_list.front();
				double dist = p.Distance(a);
				if(best_pnt_circle == NULL || dist < best_dist)
				{
					best_pnt = p;
					best_dist = dist;
					best_pnt_circle = &circle;
				}
			}
		}

		if(best_pnt_circle)
		{
			p = best_pnt;
			centre = best_pnt_circle->Location();
			axis = best_pnt_circle->Axis().Direction();
			return true;
		}
	}

	return false;
}

gp_Circ HCircle::GetCircle() const
{
	return gp_Circ(gp_Ax2(C->m_p,m_axis.Direction()),m_radius);
}

void HCircle::SetCircle(gp_Circ c)
{
	m_radius = c.Radius();
	C->m_p = c.Location();
	m_axis = c.Axis();
}

void HCircle::LoadFromDoubles()
{
	C->LoadFromDoubles();
}

void HCircle::LoadToDoubles()
{
	C->LoadToDoubles();
}

static HCircle *object_for_tools = NULL;

class ClickMidpointOfCircle: public Tool
{
public:
	void Run()
	{
		CBox box;
		object_for_tools->GetBox(box);
		double centre[3];
		box.Centre(centre);

		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(gp_Pnt(centre[0], centre[1], centre[2]), DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wxChar* GetTitle(){return _("Click midpoint");}
	wxString BitmapPath(){return _T("click_circle_midpoint");}
};

ClickMidpointOfCircle click_midpoint_of_circle;


class ClickNorthernMidpointOfCircle: public Tool
{
public:
	void Run()
	{
		CBox box;
		object_for_tools->GetBox(box);
		double centre[3];
		box.Centre(centre);		

		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(gp_Pnt(centre[0], box.MaxY(), centre[2]), DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wxChar* GetTitle(){return _("Click centre-top point");}
	wxString BitmapPath(){return _T("click_circle_centre_top");}
};

ClickNorthernMidpointOfCircle click_northern_midpoint_of_circle;

class ClickSouthernMidpointOfCircle: public Tool
{
public:
	void Run()
	{
		CBox box;
		object_for_tools->GetBox(box);
		double centre[3];
		box.Centre(centre);		

		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(gp_Pnt(centre[0], box.MinY(), centre[2]), DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wxChar* GetTitle(){return _("Click centre-bottom point");}
	wxString BitmapPath(){return _T("click_circle_centre_bottom");}
};

ClickSouthernMidpointOfCircle click_southern_midpoint_of_circle;

class ClickEasternMidpointOfCircle: public Tool
{
public:
	void Run()
	{
		CBox box;
		object_for_tools->GetBox(box);
		double centre[3];
		box.Centre(centre);		

		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(gp_Pnt(box.MaxX(), centre[1], centre[2]), DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wxChar* GetTitle(){return _("Click centre-right point");}
	wxString BitmapPath(){return _T("click_circle_centre_right");}
};

ClickEasternMidpointOfCircle click_eastern_midpoint_of_circle;

class ClickWesternMidpointOfCircle: public Tool
{
public:
	void Run()
	{
		CBox box;
		object_for_tools->GetBox(box);
		double centre[3];
		box.Centre(centre);		

		wxGetApp().m_digitizing->digitized_point = DigitizedPoint(gp_Pnt(box.MinX(), centre[1], centre[2]), DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wxChar* GetTitle(){return _("Click centre-left point");}
	wxString BitmapPath(){return _T("click_circle_centre_left");}
};

ClickWesternMidpointOfCircle click_western_midpoint_of_circle;

void HCircle::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	object_for_tools = this;

	Drawing *pDrawingMode = dynamic_cast<Drawing *>(wxGetApp().input_mode_object);
	if (pDrawingMode != NULL)
	{
		// We're drawing something.  Allow these options.
		t_list->push_back(&click_midpoint_of_circle);
		t_list->push_back(&click_northern_midpoint_of_circle);
		t_list->push_back(&click_southern_midpoint_of_circle);
		t_list->push_back(&click_eastern_midpoint_of_circle);
		t_list->push_back(&click_western_midpoint_of_circle);
	}

}