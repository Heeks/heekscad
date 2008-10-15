// CordinateSystem.cpp

#include "stdafx.h"
#include "CoordinateSystem.h"
#include "PropertyVertex.h"

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
	((CoordinateSystem*)object)->m_x = gp_Vec(pos.XYZ());
	wxGetApp().Repaint();
}

static void on_set_y(const gp_Pnt& pos, HeeksObj* object)
{
	((CoordinateSystem*)object)->m_y = gp_Vec(pos.XYZ());
	wxGetApp().Repaint();
}

void CoordinateSystem::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyVertex(_T("position"), m_o, this, on_set_pos));
	list->push_back(new PropertyVertex(_T("x axis"), gp_Pnt(m_x.XYZ()), this, on_set_x));
	list->push_back(new PropertyVertex(_T("y axis"), gp_Pnt(m_y.XYZ()), this, on_set_y));
	double v, h, t;
	AxesToAngles(m_x, m_y, v, h, t);
}

void CoordinateSystem::WriteXML(TiXmlElement *root)
{
	// to do
}

gp_Trsf CoordinateSystem::GetMatrix()
{
	return make_matrix(m_o, m_x, m_y);
}

// static
HeeksObj* CoordinateSystem::ReadFromXMLElement(TiXmlElement* pElem)
{
	// to do
	return NULL;
}

// code for AxesToAngles copied from http://tog.acm.org/GraphicsGems/gemsiv/euler_angle/EulerAngles.c

#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}

//static
void CoordinateSystem::AxesToAngles(const gp_Dir &x, const gp_Dir &y, double &v_angle, double &h_angle, double &t_angle)
{
	double M[4][4];
	extract(make_matrix(gp_Pnt(0, 0, 0), x, y), M[0]);
	int order = 0;

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
	if (cy > 16*FLT_EPSILON) {
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
}
