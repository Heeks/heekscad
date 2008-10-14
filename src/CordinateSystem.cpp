// CordinateSystem.cpp

#include "stdafx.h"
#include "CoordinateSystem.h"

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

void CoordinateSystem::glCommands(bool select, bool marked, bool no_color)
{
	// red, green, blue for x, y, z, like I saw on Open Arena
	if(this == wxGetApp().m_current_coordinate_system)glLineWidth(2);

	glPushMatrix();
	double m[16];
	extract_transposed(GetMatrix(), m);
	glMultMatrixd(m);

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

void CoordinateSystem::GetProperties(std::list<Property *> *list)
{
	// to do
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
