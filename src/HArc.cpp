// HArc.cpp
#include "stdafx.h"

#include "HArc.h"
#include "HLine.h"
#include "HILine.h"
#include "HCircle.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyChoice.h"
#include "../tinyxml/tinyxml.h"
#include "../interface/PropertyVertex.h"
#include "Gripper.h"

HArc::HArc(const HArc &line){
	operator=(line);
}

HArc::HArc(const gp_Pnt &a, const gp_Pnt &b, const gp_Circ &c, const HeeksColor* col):color(*col), A(a), B(b), m_circle(c){
}

HArc::~HArc(){
}

const HArc& HArc::operator=(const HArc &b){
	HeeksObj::operator=(b);
	A = b.A;
	B = b.B;
	m_circle = b.m_circle;
	color = b.color;
	return *this;
}



//segments - number of segments per full revolution!
//d_angle - determines the direction and the ammount of the arc to draw
void HArc::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{
	if(A.IsEqual(B, wxGetApp().m_geom_tol)){
		return;
	}
	gp_Dir x_axis = m_circle.XAxis().Direction();
	gp_Dir y_axis = m_circle.YAxis().Direction();
	gp_Pnt centre = m_circle.Location();

	double ax = gp_Vec(A.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B.XYZ() - centre.XYZ()) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(start_angle > end_angle)end_angle += 6.28318530717958;

	double radius = m_circle.Radius();
	double d_angle = end_angle - start_angle;
	int segments = (int)(fabs(pixels_per_mm * radius * d_angle / 6.28318530717958 + 1));
    
    double theta = d_angle / (double)segments;
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
}

void HArc::Draw(wxDC& dc)
{
	wxGetApp().PlotSetColor(color);
	double s[3], e[3], c[3];
	extract(A, s);
	extract(B, e);
	extract(m_circle.Location(), c);
	wxGetApp().PlotArc(s, e, c);
}

HeeksObj *HArc::MakeACopy(void)const{
		HArc *new_object = new HArc(*this);
		return new_object;
}

bool HArc::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A.Transform(mat);
	B.Transform(mat);
	m_circle.Transform(mat);
	return false;
}

void HArc::GetBox(CBox &box){
	box.Insert(A.X(), A.Y(), A.Z());
	box.Insert(B.X(), B.Y(), B.Z());
}

void HArc::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	list->push_back(GripperTypeStretch);
	list->push_back(A.X());
	list->push_back(A.Y());
	list->push_back(A.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(B.X());
	list->push_back(B.Y());
	list->push_back(B.Z());
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
	((HArc*)object)->m_circle.SetLocation(make_point(vt));
	wxGetApp().Repaint();
}

static void on_set_axis(const double *vt, HeeksObj* object){
	gp_Ax1 a = ((HArc*)object)->m_circle.Axis();
	a.SetDirection(make_vector(vt));
	((HArc*)object)->m_circle.SetAxis(a);
	wxGetApp().Repaint();
}

void HArc::GetProperties(std::list<Property *> *list){
	double a[3], b[3];
	double c[3], ax[3];
	extract(A, a);
	extract(B, b);
	extract(m_circle.Location(), c);
	extract(m_circle.Axis().Direction(), ax);
	list->push_back(new PropertyVertex(_("start"), a, this, on_set_start));
	list->push_back(new PropertyVertex(_("end"), b, this, on_set_end));
	list->push_back(new PropertyVertex(_("centre"), c, this, on_set_centre));
	list->push_back(new PropertyVertex(_("axis"), ax, this, on_set_axis));
	double length = A.Distance(B);
	list->push_back(new PropertyDouble(_("Length"), length, NULL));

	HeeksObj::GetProperties(list);
}

int HArc::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
	int numi = 0;

	switch(object->GetType())
	{
	case LineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HLine*)object)->GetLine(), m_circle, plist);
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
			intersect(((HILine*)object)->GetLine(), m_circle, plist);
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
			intersect(m_circle, ((HArc*)object)->m_circle, plist);
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
			intersect(m_circle, ((HCircle*)object)->m_circle, plist);
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

bool HArc::Intersects(const gp_Pnt &pnt)const
{
	if(!intersect(pnt, m_circle))return false;

	if(pnt.IsEqual(A, wxGetApp().m_geom_tol)){
		return true;
	}

	if(pnt.IsEqual(B, wxGetApp().m_geom_tol)){
		return true;
	}

	if(A.IsEqual(B, wxGetApp().m_geom_tol)){
		return false; // no size arc!
	}

	gp_Dir x_axis = m_circle.XAxis().Direction();
	gp_Dir y_axis = m_circle.YAxis().Direction();
	gp_Pnt centre = m_circle.Location();

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
	ClosestPointsLineAndCircle(ray, m_circle, rl);
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

bool HArc::Stretch(const double *p, const double* shift){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(A.IsEqual(vp, wxGetApp().m_geom_tol)){
		A = A.XYZ() + vshift.XYZ();
	}
	else if(B.IsEqual(vp, wxGetApp().m_geom_tol)){
		gp_Vec direction = GetSegmentVector(0.0);
		gp_Pnt centre;
		gp_Dir axis;
		gp_Pnt new_B = gp_Pnt(B.XYZ() + vshift.XYZ());
		if(HArc::TangentialArc(A, direction, new_B, centre, axis))
		{
			m_circle.SetAxis(gp_Ax1(centre, axis));
			m_circle.SetRadius(A.Distance(centre));
			B = new_B;
		}
	}
	return false;
}

bool HArc::GetStartPoint(double* pos)
{
	extract(A, pos);
	return true;
}

bool HArc::GetEndPoint(double* pos)
{
	extract(B, pos);
	return true;
}

bool HArc::GetCentrePoint(double* pos)
{
	extract(m_circle.Location(), pos);
	return true;
}

gp_Vec HArc::GetSegmentVector(double fraction)
{
	gp_Pnt centre = m_circle.Location();
	gp_Pnt p = GetPointAtFraction(fraction);
	gp_Vec vp(centre, p);
	gp_Vec vd = gp_Vec(m_circle.Axis().Direction()) ^ vp;
	vd.Normalize();
	return vd;
}

gp_Pnt HArc::GetPointAtFraction(double fraction)
{
	if(A.IsEqual(B, wxGetApp().m_geom_tol)){
		return A;
	}
	gp_Dir x_axis = m_circle.XAxis().Direction();
	gp_Dir y_axis = m_circle.YAxis().Direction();
	gp_Pnt centre = m_circle.Location();

	double ax = gp_Vec(A.XYZ() - centre.XYZ()) * x_axis;
	double ay = gp_Vec(A.XYZ() - centre.XYZ()) * y_axis;
	double bx = gp_Vec(B.XYZ() - centre.XYZ()) * x_axis;
	double by = gp_Vec(B.XYZ() - centre.XYZ()) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(start_angle > end_angle)end_angle += 6.28318530717958;

	double radius = m_circle.Radius();
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
	element->SetDoubleAttribute("sx", A.X());
	element->SetDoubleAttribute("sy", A.Y());
	element->SetDoubleAttribute("sz", A.Z());
	element->SetDoubleAttribute("ex", B.X());
	element->SetDoubleAttribute("ey", B.Y());
	element->SetDoubleAttribute("ez", B.Z());
	gp_Pnt C = m_circle.Location();
	gp_Dir D = m_circle.Axis().Direction();
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
	gp_Pnt p0, p1, centre;
	double axis[3];
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
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

	return new_object;
}

void HArc::Reverse()
{
	gp_Pnt temp = A;
	A = B;
	B = temp;
	m_circle.SetAxis(m_circle.Axis().Reversed());
}