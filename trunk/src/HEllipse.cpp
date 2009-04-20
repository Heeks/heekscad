// HEllipse.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HEllipse.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyChoice.h"
#include "../tinyxml/tinyxml.h"
#include "../interface/PropertyVertex.h"
#include "HLine.h"
#include "HILine.h"
#include "HArc.h"
#include "Gripper.h"

HEllipse::HEllipse(const HEllipse &e){
	operator=(e);
}

HEllipse::HEllipse(const gp_Elips &e, const HeeksColor* col):color(*col), m_ellipse(e){
}

HEllipse::~HEllipse(){
}

const HEllipse& HEllipse::operator=(const HEllipse &e){
	HeeksObj::operator=(e);
	m_ellipse = e.m_ellipse;
	color = e.color;
	return *this;
}



//segments - number of segments per full revolution!
void HEllipse::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{
	gp_Dir x_axis = m_ellipse.XAxis().Direction();
	gp_Dir y_axis = m_ellipse.YAxis().Direction();
	gp_Pnt centre = m_ellipse.Location();

	double radius = m_ellipse.MajorRadius();
	double min_radius = m_ellipse.MinorRadius();
	int segments = (int)(fabs(pixels_per_mm * radius + 1));
    
    double theta = 6.28318530717958 / (double)segments;
    double tangetial_factor = tan(theta);
    double radial_factor = 1 - cos(theta);
    
    double x = radius;
    double y = 0.0;

	double pp[3];

   for(int i = 0; i < segments + 1; i++)
    {
		gp_Pnt p = centre.XYZ() + x * x_axis.XYZ() + y * y_axis.XYZ()  * min_radius/radius;
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

void HEllipse::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		wxGetApp().glColorEnsuringContrast(color);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(3, 0xaaaa);
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
		glDisable(GL_LINE_STIPPLE);
	}
}

HeeksObj *HEllipse::MakeACopy(void)const{
		HEllipse *new_object = new HEllipse(*this);
		return new_object;
}

bool HEllipse::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	m_ellipse.Transform(mat);
	return false;
}

void HEllipse::GetBox(CBox &box){
	gp_Dir x_axis = m_ellipse.XAxis().Direction();
	gp_Dir y_axis = m_ellipse.YAxis().Direction();
	gp_XYZ c = m_ellipse.Location().XYZ();
	gp_XYZ x = x_axis.XYZ() * m_ellipse.MajorRadius();
	gp_XYZ y = y_axis.XYZ() * m_ellipse.MinorRadius();

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

void HEllipse::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	if(!just_for_endof)
	{
		gp_Dir x_axis = m_ellipse.XAxis().Direction();
		gp_Dir y_axis = m_ellipse.YAxis().Direction();
		gp_XYZ c = m_ellipse.Location().XYZ();
		double maj_r = m_ellipse.MajorRadius();
		double min_r = m_ellipse.MinorRadius();
		gp_Pnt maj_s(c + x_axis.XYZ() * maj_r);
		gp_Pnt min_s(c + y_axis.XYZ() * min_r);

		list->push_back(GripperTypeStretch);
		list->push_back(maj_s.X());
		list->push_back(maj_s.Y());
		list->push_back(maj_s.Z());

		list->push_back(GripperTypeStretch);
		list->push_back(min_s.X());
		list->push_back(min_s.Y());
		list->push_back(min_s.Z());
	} 
}

static void on_set_centre(const double *vt, HeeksObj* object){
	((HEllipse*)object)->m_ellipse.SetLocation(make_point(vt));
	wxGetApp().Repaint();
}

static void on_set_axis(const double *vt, HeeksObj* object){
	gp_Ax1 a = ((HEllipse*)object)->m_ellipse.Axis();
	a.SetDirection(make_vector(vt));
	((HEllipse*)object)->m_ellipse.SetAxis(a);
	wxGetApp().Repaint(); 
}

static void on_set_major_radius(double value, HeeksObj* object){
	((HEllipse*)object)->m_ellipse.SetMajorRadius(value);
	wxGetApp().Repaint(); 
}

static void on_set_minor_radius(double value, HeeksObj* object){
	((HEllipse*)object)->m_ellipse.SetMinorRadius(value);
	wxGetApp().Repaint(); 
}

static void on_set_rotation(double value, HeeksObj* object){
	gp_Dir up(0, 0, 1);
        double rot = ((HEllipse*)object)->GetRotation();
	((HEllipse*)object)->m_ellipse.Rotate(gp_Ax1(((HEllipse*)object)->m_ellipse.Location(),up),value-rot);
	wxGetApp().Repaint(); 
}

double HEllipse::GetRotation()
{
	double x = m_ellipse.YAxis().Direction().X();
	double y = m_ellipse.YAxis().Direction().Y();
	return atan2(y,x);
}

void HEllipse::GetProperties(std::list<Property *> *list){
	double c[3], a[3];
	extract(m_ellipse.Location(), c);
	extract(m_ellipse.Axis().Direction(), a);
	double rot = GetRotation();
	list->push_back(new PropertyVertex(_("centre"), c, this, on_set_centre));
	list->push_back(new PropertyVertex(_("axis"), a, this, on_set_axis));
	list->push_back(new PropertyDouble(_("major radius"), m_ellipse.MajorRadius(), this, on_set_major_radius));
	list->push_back(new PropertyDouble(_("minor radius"), m_ellipse.MinorRadius(), this, on_set_minor_radius));
	list->push_back(new PropertyDouble(_("rotation"), rot, this, on_set_rotation));
	HeeksObj::GetProperties(list);
}

bool HEllipse::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
/*	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< gp_Pnt > rl;
	ClosestPointsLineAndCircle(ray, m_circle, rl);
	if(rl.size()>0)
	{
		extract(rl.front(), point);
		return true;
	}

	return false; */
}

bool HEllipse::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this circle is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

bool HEllipse::Stretch(const double *p, const double* shift){

	//TODO: 
	// 1. It is difficult to shrink the inner radius and enlarge the outer radius. 
	// 2. The handle switches to the other radius if you go past Pi/4
        // 3. Need to make sure that major radius is always larger than minor
        //    Probably should just internally switch them and rotate the shape 90 degrees

	gp_Pnt vp = make_point(p);
	gp_Pnt zp(0,0,0);
	gp_Dir up(0, 0, 1);
	gp_Vec vshift = make_vector(shift);

	double rot = GetRotation();

	gp_Dir x_axis = m_ellipse.XAxis().Direction();
	gp_Dir y_axis = m_ellipse.YAxis().Direction();
	gp_Pnt c = m_ellipse.Location();
	double maj_r = m_ellipse.MajorRadius();
	double min_r = m_ellipse.MinorRadius();
	gp_Pnt maj_s(c.XYZ() + x_axis.XYZ() * maj_r);
	gp_Pnt min_s(c.XYZ() + y_axis.XYZ() * min_r);

        double d = c.Distance(vp);
        double f = DistanceToFoci(vp)/2;

	if(c.IsEqual(vp, wxGetApp().m_geom_tol)){
		m_ellipse.SetLocation(c.XYZ() + vshift.XYZ());
	}
        else if(f < m_ellipse.MajorRadius() +  wxGetApp().m_geom_tol && d > m_ellipse.MinorRadius() -  wxGetApp().m_geom_tol)
	{
		//We have to rotate the incoming vector to be in our coordinate system
		gp_Pnt cir = vp.XYZ() - c.XYZ();
		cir.Rotate(gp_Ax1(zp,up),-rot);

		//This is shockingly simple
		if( d > m_ellipse.MinorRadius() + (m_ellipse.MajorRadius() - m_ellipse.MinorRadius())/2)
		{
			m_ellipse.SetMajorRadius(1/sqrt((1-(1/min_r)*(1/min_r)*cir.X()*cir.X()) / cir.Y() / cir.Y())); 
		}
		else
		{
			m_ellipse.SetMinorRadius(1/sqrt((1-(1/maj_r)*(1/maj_r)*cir.Y()*cir.Y()) / cir.X() / cir.X())); 
		}
	}
	return false; 
}

bool HEllipse::GetCentrePoint(double* pos)
{
	extract(m_ellipse.Location(), pos);
	return true;
}

void HEllipse::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Circle" );
	root->LinkEndChild( element );  
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("maj", m_ellipse.MajorRadius());
	element->SetDoubleAttribute("min", m_ellipse.MinorRadius());
	gp_Pnt C = m_ellipse.Location();
	gp_Dir D = m_ellipse.Axis().Direction();
	element->SetDoubleAttribute("cx", C.X());
	element->SetDoubleAttribute("cy", C.Y());
	element->SetDoubleAttribute("cz", C.Z());
	element->SetDoubleAttribute("ax", D.X());
	element->SetDoubleAttribute("ay", D.Y());
	element->SetDoubleAttribute("az", D.Z());
	WriteBaseXML(element); 
}

// static member function
HeeksObj* HEllipse::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt centre;
	double axis[3];
	double maj = 0.0;
	double min = 0.0;
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "maj"){maj = a->DoubleValue();}
		else if(name == "mIN"){min = a->DoubleValue();}
		else if(name == "cx"){centre.SetX(a->DoubleValue());}
		else if(name == "cy"){centre.SetY(a->DoubleValue());}
		else if(name == "cz"){centre.SetZ(a->DoubleValue());}
		else if(name == "ax"){axis[0] = a->DoubleValue();}
		else if(name == "ay"){axis[1] = a->DoubleValue();}
		else if(name == "az"){axis[2] = a->DoubleValue();}
	}

	gp_Elips ellipse(gp_Ax2(centre, gp_Dir(make_vector(axis))), maj,min);

	HEllipse* new_object = new HEllipse(ellipse, &c);
	new_object->ReadBaseXML(pElem);

	return new_object; 
}

double HEllipse::DistanceToFoci(gp_Pnt &pnt)
{
   //Returns:
   // 2*Major_Radius() if pnt is on the ellipse. 
   //<2*Major_Radius if it is inside 
   //>2*Major_Radius if it is outside

   //Pnt must be coplaner to the ellipse else it won't work. Right now math computes above for elliptic solids.
   double e = m_ellipse.Eccentricity();
   gp_Pnt f1;
   gp_Pnt f2;
   f1 = m_ellipse.Location().XYZ() + m_ellipse.XAxis().Direction().XYZ() * e;
   f2 = m_ellipse.Location().XYZ() - m_ellipse.XAxis().Direction().XYZ() * e;
   return f1.Distance(pnt) + f2.Distance(pnt);
}

int HEllipse::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
/*	int numi = 0;

	switch(object->GetType())
	{
	case LineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HLine*)object)->GetLine(), m_circle, plist);
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
			intersect(((HILine*)object)->GetLine(), m_circle, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;

	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(m_circle, ((HArc*)object)->m_circle, plist);
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
			intersect(m_circle, ((HEllipse*)object)->m_circle, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;
	}

	return numi; */
}

//static
bool HEllipse::GetLineTangentPoints(const gp_Elips& c1, const gp_Elips& c2, const gp_Pnt& a, const gp_Pnt& b, gp_Pnt& p1, gp_Pnt& p2)
{
/*	// find the tangent points for a line between two circles

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

	return true; */
}

// static
bool HEllipse::GetLineTangentPoint(const gp_Elips& c, const gp_Pnt& a, const gp_Pnt& b, gp_Pnt& p)
{
/*	// find the tangent point for a line from near point "a" on the circle to point "b"

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
*/
	return true;
}

// static
bool HEllipse::GetArcTangentPoints(const gp_Elips& c, const gp_Lin &line, const gp_Pnt& p, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis)
{
/*	// find the arc that fits tangentially from the line to the circle
	// returns p1 - on the line, p2 - on the circle, centre - the centre point of the arc

	// they need to be flat ( compared with each other )
	if(fabs(gp_Vec(c.Axis().Direction()) * gp_Vec(line.Direction())) > 0.00000001)return false;
	if(fabs( gp_Vec(c.Location().XYZ()) * c.Axis().Direction() - gp_Vec(line.Location().XYZ()) * c.Axis().Direction()) > 0.00000001)return false;

	// make a concentric circle
	bool p_infront_of_centre = gp_Vec(p.XYZ()) * gp_Vec(line.Direction()) > gp_Vec(c.Location().XYZ()) * gp_Vec(line.Direction());
	double R = p_infront_of_centre ? (c.Radius() - radius) : (c.Radius() + radius);
	if(R < 0.000000000000001)return false;
	gp_Elips concentric_circle(gp_Ax2(c.Axis().Location(), c.Axis().Direction()), R);

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
			if(best_pnt == NULL || p_infront_of_centre && dp > best_dp || !p_infront_of_centre && dp < best_dp)
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
	gp_Elips circle_for_arc(gp_Ax2(centre, c.Axis().Direction()), radius);

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

	return true; */
}

class two_circles{
public:
	gp_Elips m_c1;
	gp_Elips m_c2;

	two_circles(const gp_Elips& c1, const gp_Elips& c2)
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
/*	const two_points* best_pair = NULL;
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

	return best_pair; */
}

// static
bool HEllipse::GetArcTangentPoints(const gp_Elips& c1, const gp_Elips &c2, const gp_Pnt& a, const gp_Pnt& b, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis)
{
/*	// find the arc that fits tangentially from the circle to the circle
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

	std::list<gp_Elips> c1_list, c2_list;
	c1_list.push_back(gp_Elips(gp_Ax2(c1.Location(), c1.Axis().Direction()), r1 + radius));
	if(radius > r1)c1_list.push_back(gp_Elips(gp_Ax2(c1.Location(), c1.Axis().Direction()), radius - r1));
	c2_list.push_back(gp_Elips(gp_Ax2(c2.Location(), c2.Axis().Direction()), r2 + radius));
	if(radius > r2)c2_list.push_back(gp_Elips(gp_Ax2(c2.Location(), c2.Axis().Direction()), radius - r2));

	std::list<two_circles> combinations;
	for(std::list<gp_Elips>::iterator It1 = c1_list.begin(); It1 != c1_list.end(); It1++)
	{
		for(std::list<gp_Elips>::iterator It2 = c2_list.begin(); It2 != c2_list.end(); It2++)
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
		gp_Elips arc_circle(gp_Ax2(pnt, c1.Axis().Direction()), radius);
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
*/
	return false;
}

// static
bool HEllipse::GetArcTangentPoints(const gp_Lin& l1, const gp_Lin &l2, const gp_Pnt& a, const gp_Pnt& b, double radius, gp_Pnt& p1, gp_Pnt& p2, gp_Pnt& centre, gp_Dir& axis)
{
/*	// cross product to find axis of arc
	gp_Vec xp = gp_Vec(l1.Direction()) ^ gp_Vec(l2.Direction());
	if(xp.Magnitude() < 0.00000000000001)return false;
	axis = gp_Dir(xp);

	gp_Vec left1 = axis ^ l1.Direction();
	gp_Vec left2 = axis ^ l2.Direction();

	gp_Lin offset_l1_left(l1.Location().XYZ() + left1.XYZ() * radius, l1.Direction());
	gp_Lin offset_l1_right(l1.Location().XYZ() - left1.XYZ() * radius, l1.Direction());
	gp_Lin offset_l2_left(l2.Location().XYZ() + left2.XYZ() * radius, l2.Direction());
	gp_Lin offset_l2_right(l2.Location().XYZ() - left2.XYZ() * radius, l2.Direction());

	std::list<gp_Elips> circles;

	gp_Pnt pnt;
	if(intersect(offset_l1_left, offset_l2_left, pnt))
		circles.push_back(gp_Elips(gp_Ax2(pnt, axis), radius));
	if(intersect(offset_l1_left, offset_l2_right, pnt))
		circles.push_back(gp_Elips(gp_Ax2(pnt, axis), radius));
	if(intersect(offset_l1_right, offset_l2_left, pnt))
		circles.push_back(gp_Elips(gp_Ax2(pnt, axis), radius));
	if(intersect(offset_l1_right, offset_l2_right, pnt))
		circles.push_back(gp_Elips(gp_Ax2(pnt, axis), radius));

	std::list<two_points> point_pairs;

	for(std::list<gp_Elips>::iterator It = circles.begin(); It != circles.end(); It++)
	{
		gp_Elips& c = *It;
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
*/
	return false;
}

// static
bool HEllipse::GetArcTangentPoint(const gp_Lin& l, const gp_Pnt& a, const gp_Pnt& b, const gp_Vec *final_direction, double* radius, gp_Pnt& p, gp_Pnt& centre, gp_Dir& axis)
{
	// find the tangent point on the line l, for an arc from near point "a" on the line to exact given point "b"
/*
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
							intersect(l, gp_Elips(gp_Ax2(centre, axis), R), plist);
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
		gp_Elips offset_circle(gp_Ax2(b, axis), radius_to_use);
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
			intersect(l, gp_Elips(gp_Ax2(centre, axis), radius_to_use), rl);
			if(rl.size() == 1)
			{
				p = rl.front();
				return true;
			}
		}
	}
*/
	return false;
}

// static
bool HEllipse::GetArcTangentPoint(const gp_Elips& c, const gp_Pnt& a, const gp_Pnt& b, const gp_Vec *final_direction, double* radius, gp_Pnt& p, gp_Pnt& centre, gp_Dir& axis)
{
	// find the tangent point on the circle c, for an arc from near point "a" on the circle to exact given point "b"
/*	if(final_direction)
	{
		// get tangent circles
		gp_Lin final_dir_line(b, gp_Dir(final_direction->XYZ()));
		std::list<gp_Elips> c_list;
		TangentCircles(c, final_dir_line, b, c_list);

		gp_Elips* best_pnt_circle = NULL;
		gp_Pnt best_pnt;
		double best_dist = 0.0;

		for(std::list<gp_Elips>::iterator It = c_list.begin(); It != c_list.end(); It++)
		{
			gp_Elips& circle = *It;
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
*/
	return false;
}
