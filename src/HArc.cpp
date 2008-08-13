// HArc.cpp
#include "stdafx.h"

#include "HArc.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyChoice.h"
#include "../tinyxml/tinyxml.h"
#include "PropertyVertex.h"

HArc::HArc(const HArc &line){
	operator=(line);
}

HArc::HArc(const gp_Pnt &a, const gp_Pnt &b, const gp_Circ &c, const HeeksColor* col):A(a), B(b), m_circle(c), color(*col){
}

HArc::~HArc(){
}

const HArc& HArc::operator=(const HArc &b){
	HGeomObject::operator=(b);
	A = b.A;
	B = b.B;
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
	int segments = fabs(pixels_per_mm * radius * d_angle / 6.28318530717958 + 1);
    
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

HeeksObj *HArc::MakeACopy(void)const{
		HArc *new_object = new HArc(*this);
		return new_object;
}

void HArc::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A.Transform(mat);
	B.Transform(mat);
	m_circle.Transform(mat);
}

void HArc::GetBox(CBox &box){
	box.Insert(A.X(), A.Y(), A.Z());
	box.Insert(B.X(), B.Y(), B.Z());
}

void HArc::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	list->push_back(0);
	list->push_back(A.X());
	list->push_back(A.Y());
	list->push_back(A.Z());
	list->push_back(0);
	list->push_back(B.X());
	list->push_back(B.Y());
	list->push_back(B.Z());
}

static HArc* arc_for_properties = NULL;
static void on_set_start(const gp_Pnt &vt){
	arc_for_properties->A = vt;
	wxGetApp().Repaint();
}

static void on_set_end(const gp_Pnt &vt){
	arc_for_properties->B = vt;
	wxGetApp().Repaint();
}

static void on_set_centre(const gp_Pnt &vt){
	arc_for_properties->m_circle.SetLocation(vt);
	wxGetApp().Repaint();
}

static void on_set_axis(const gp_Pnt &vt){
	gp_Ax1 a = arc_for_properties->m_circle.Axis();
	a.SetDirection(gp_Dir(vt.XYZ()));
	arc_for_properties->m_circle.SetAxis(a);
	wxGetApp().Repaint();
}

void HArc::GetProperties(std::list<Property *> *list){
	__super::GetProperties(list);

	arc_for_properties = this;
	list->push_back(new PropertyVertex("start", A, on_set_start));
	list->push_back(new PropertyVertex("end", B, on_set_end));
	list->push_back(new PropertyVertex("centre", m_circle.Location(), on_set_centre));
	list->push_back(new PropertyVertex("axis", gp_Pnt(m_circle.Axis().Direction().XYZ()), on_set_axis));
	double length = A.Distance(B);
	list->push_back(new PropertyDouble("Length", length, NULL));
}

bool HArc::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	// to do
	return false;
}

void HArc::Stretch(const double *p, const double* shift, double* new_position){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(A.IsEqual(vp, wxGetApp().m_geom_tol)){
		A = A.XYZ() + vshift.XYZ();
		extract(A, new_position);
	}
	else if(B.IsEqual(vp, wxGetApp().m_geom_tol)){
		B = B.XYZ() + vshift.XYZ();
		extract(B, new_position);
	}
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
bool HArc::TangentialArc(const gp_Pnt &p0, const gp_Vec &v0, const gp_Pnt &p1, gp_Pnt &centre, gp_Vec &axis)
{
	// returns false if a straight line is needed 
	// else returns true and sets centre and axis
	std::list<gp_Pnt> rl;
	gp_Dir direction_for_circle;
	if(p0.Distance(p1) > 0.0000000001 && v0.Magnitude() > 0.0000000001){
		gp_Vec v1(p0, p1);
		gp_Pnt halfway(p0.XYZ() + v1.XYZ() * 0.5);
		gp_Pln pl1(halfway, v1);
		gp_Pln pl2(p0, v0);
		gp_Lin plane_line;
		intersect(pl1, pl2, plane_line);
		direction_for_circle = -(plane_line.Direction());
		gp_Lin l1(halfway, v1);
		intersect(plane_line, l1, rl);
	}

	if(rl.size() == 0){
		// line
		return false;
	}
	else{
		// arc
		centre = rl.front();
		axis = direction_for_circle;
		return true;
	}
}

void HArc::WriteXML(TiXmlElement *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Arc" );
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
		wxString name(a->Name());
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

	return new HArc(p0, p1, circle, &c);
}