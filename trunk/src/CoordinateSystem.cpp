// CordinateSystem.cpp

#include "stdafx.h"
#include "CoordinateSystem.h"
#include "PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "HeeksFrame.h"
#include "ObjPropsCanvas.h"
#include "../tinyxml/tinyxml.h"

wxIcon* CoordinateSystem::m_icon = NULL;

wxIcon* CoordinateSystem::GetIcon()
{
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + _T("/icons/coordsys.png"), wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}

CoordinateSystem::CoordinateSystem(const wxString& str, const gp_Pnt &o, const gp_Dir &x, const gp_Dir &y)
{
	m_title = str;
	m_o = o;
	m_x = x;
	m_y = y;
}

CoordinateSystem::CoordinateSystem(const CoordinateSystem &c)
{
	operator=(c);
}

const CoordinateSystem& CoordinateSystem::operator=(const CoordinateSystem &c)
{
	HeeksObj::operator=(c);
	m_o = c.m_o;
	m_x = c.m_x;
	m_y = c.m_y;
	return *this;
}

CoordinateSystem::~CoordinateSystem(void)
{
}

// static
void CoordinateSystem::RenderDatum()
{
	double size = 30.0 / wxGetApp().GetPixelScale(); // 30 pixels as mm
	glBegin(GL_LINES);
	glColor3ub(255, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(size, 0, 0);
	glColor3ub(0, 255, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, size, 0);
	glColor3ub(0, 0, 255);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, size);
	glEnd();
}

void CoordinateSystem::glCommands(bool select, bool marked, bool no_color)
{
	// red, green, blue for x, y, z, like I saw on Open Arena
	if(this == wxGetApp().m_current_coordinate_system)glLineWidth(2);

	glPushMatrix();
	double m[16];
	extract_transposed(GetMatrix(), m);
	glMultMatrixd(m);
	RenderDatum();
	glPopMatrix();
	glLineWidth(1);
}

void CoordinateSystem::GetBox(CBox &box)
{
	gp_Pnt vt(0, 0, 0);
	vt.Transform(GetMatrix());
	double p[3];
	extract(vt, p);
	box.Insert(p);
}

HeeksObj *CoordinateSystem::MakeACopy(void)const
{
	return new CoordinateSystem(*this);
}

bool CoordinateSystem::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_o.Transform(mat);
	m_x.Transform(mat);
	m_y.Transform(mat);
	return false;
}

static void on_set_pos(const gp_Pnt& pos, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_o = pos;
	wxGetApp().Repaint();
}

static void on_set_x(const gp_Pnt& pos, HeeksObj* object)
{
//	((CoordinateSystem*)object)->m_x = gp_Vec(pos.XYZ());
//	((CoordinateSystem*)object)->AxesToAngles();
//	wxGetApp().Repaint();
}

static void on_set_y(const gp_Pnt& pos, HeeksObj* object)
{
//	((CoordinateSystem*)object)->m_y = gp_Vec(pos.XYZ());
//	((CoordinateSystem*)object)->AxesToAngles();
//	wxGetApp().Repaint();
}

static void on_set_vertical_angle(double value, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_vertical_angle = value * Pi/180;
	((CoordinateSystem*)object)->AnglesToAxes();
	wxGetApp().Repaint();
}

static void on_set_horizontal_angle(double value, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_horizontal_angle = value * Pi/180;
	((CoordinateSystem*)object)->AnglesToAxes();
	wxGetApp().Repaint();
}

static void on_set_twist_angle(double value, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_twist_angle = value * Pi/180;
	((CoordinateSystem*)object)->AnglesToAxes();
	wxGetApp().Repaint();
}

void CoordinateSystem::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyVertex(_T("position"), m_o, this, on_set_pos));
	list->push_back(new PropertyVertex(_T("x axis"), gp_Pnt(m_x.XYZ()), this, on_set_x));
	list->push_back(new PropertyVertex(_T("y axis"), gp_Pnt(m_y.XYZ()), this, on_set_y));
	AxesToAngles(m_x, m_y, m_vertical_angle, m_horizontal_angle, m_twist_angle);
	list->push_back(new PropertyDouble(_T("Vertical Angle"), m_vertical_angle * 180/Pi, this, on_set_vertical_angle));
	list->push_back(new PropertyDouble(_T("Horizontal Angle"), m_horizontal_angle * 180/Pi, this, on_set_horizontal_angle));
	list->push_back(new PropertyDouble(_T("Twist Angle"), m_twist_angle * 180/Pi, this, on_set_twist_angle));
}

void CoordinateSystem::WriteXML(TiXmlElement *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "CoordinateSystem" );
	root->LinkEndChild( element );  
	element->SetAttribute("title", m_title.c_str());
	element->SetDoubleAttribute("ox", m_o.X());
	element->SetDoubleAttribute("oy", m_o.Y());
	element->SetDoubleAttribute("oz", m_o.Z());
	element->SetDoubleAttribute("xx", m_x.X());
	element->SetDoubleAttribute("xy", m_x.Y());
	element->SetDoubleAttribute("xz", m_x.Z());
	element->SetDoubleAttribute("yx", m_y.X());
	element->SetDoubleAttribute("yy", m_y.Y());
	element->SetDoubleAttribute("yz", m_y.Z());
	WriteBaseXML(element);
}

gp_Trsf CoordinateSystem::GetMatrix()
{
	return make_matrix(m_o, m_x, m_y);
}

// static
HeeksObj* CoordinateSystem::ReadFromXMLElement(TiXmlElement* pElem)
{
	gp_Pnt o;
	gp_Dir x, y;
	wxString title;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "title"){title.assign(a->Value());}
		else if(name == "ox"){o.SetX(a->DoubleValue());}
		else if(name == "oy"){o.SetY(a->DoubleValue());}
		else if(name == "oz"){o.SetZ(a->DoubleValue());}
		else if(name == "xx"){x.SetX(a->DoubleValue());}
		else if(name == "xy"){x.SetY(a->DoubleValue());}
		else if(name == "xz"){x.SetZ(a->DoubleValue());}
		else if(name == "yx"){y.SetX(a->DoubleValue());}
		else if(name == "yy"){y.SetY(a->DoubleValue());}
		else if(name == "yz"){y.SetZ(a->DoubleValue());}
	}

	CoordinateSystem* new_object = new CoordinateSystem(title, o, x, y);
	new_object->ReadBaseXML(pElem);

	return new_object;
}

// code for AxesToAngles copied from http://tog.acm.org/GraphicsGems/gemsiv/euler_angle/EulerAngles.c

#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
#define EulOrdZXZs    EulOrd(2,0,1,0)

//static
void CoordinateSystem::AxesToAngles(const gp_Dir &x, const gp_Dir &y, double &v_angle, double &h_angle, double &t_angle)
{
	double M[4][4];
	extract(make_matrix(gp_Pnt(0, 0, 0), x, y), M[0]);
	int order = EulOrdZXZs;

    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==1) {
	double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);
	if (sy > 16*FLT_EPSILON) {
	    t_angle = atan2(M[i][j], M[i][k]);
	    v_angle = atan2(sy, M[i][i]);
	    h_angle = atan2(M[j][i], -M[k][i]);
	} else {
	    t_angle = atan2(-M[j][k], M[j][j]);
	    v_angle = atan2(sy, M[i][i]);
	    h_angle = 0;
	}
    } else {
	double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);
	if (cy > 16*DBL_EPSILON) {
	    t_angle = atan2(M[k][j], M[k][k]);
	    v_angle = atan2(-M[k][i], cy);
	    h_angle = atan2(M[j][i], M[i][i]);
	} else {
	    t_angle = atan2(-M[j][k], M[j][j]);
	    v_angle = atan2(-M[k][i], cy);
	    h_angle = 0;
	}
    }
    if (n==1) {t_angle = -t_angle; v_angle = - v_angle; h_angle = -h_angle;}
    if (f==1) {double t = t_angle; t_angle = h_angle; h_angle = t;}
}

//static
void CoordinateSystem::AnglesToAxes(const double &v_angle, const double &h_angle, const double &t_angle, gp_Dir &x, gp_Dir &y)
{
	gp_Trsf zmat1;
	zmat1.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), t_angle);

	gp_Trsf xmat;
	xmat.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)), v_angle);

	gp_Trsf zmat2;
	zmat2.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), h_angle);

	gp_Trsf mat = zmat2 * xmat * zmat1;

	x = gp_Dir(1, 0, 0).Transformed(mat);
	y = gp_Dir(0, 1, 0).Transformed(mat);
}
