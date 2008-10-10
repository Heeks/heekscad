// Solid.cpp
#include "stdafx.h"
#include "Solid.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>

wxIcon* CSphere::m_icon = NULL;

CSphere::CSphere(const gp_Pnt& pos, double radius):m_pos(pos), m_radius(radius), CSolid(BRepPrimAPI_MakeSphere(pos, radius), _T("Sphere"))
{
}

wxIcon* CSphere::GetIcon()
{
	if(m_icon == NULL){wxString exe_folder = wxGetApp().GetExeFolder();	m_icon = new wxIcon(exe_folder + _T("/icons/sphere.png"), wxBITMAP_TYPE_PNG);}
	return m_icon;
}

HeeksObj *CSphere::MakeACopy(void)const
{
	return new CSphere(*this);
}


wxIcon* CCuboid::m_icon = NULL;

CCuboid::CCuboid(const gp_Ax2& pos, double x, double y, double z):m_pos(pos), m_x(x), m_y(y), m_z(z), CSolid(BRepPrimAPI_MakeBox(pos, x, y, z), _T("Cuboid"))
{
}

wxIcon* CCuboid::GetIcon()
{
	if(m_icon == NULL){wxString exe_folder = wxGetApp().GetExeFolder();	m_icon = new wxIcon(exe_folder + _T("/icons/cube.png"), wxBITMAP_TYPE_PNG);}
	return m_icon;
}

HeeksObj *CCuboid::MakeACopy(void)const
{
	return new CCuboid(*this);
}

wxIcon* CCylinder::m_icon = NULL;

CCylinder::CCylinder(const gp_Ax2& pos, double radius, double height):m_pos(pos), m_radius(radius), m_height(height), CSolid(BRepPrimAPI_MakeCylinder(pos, radius, height), _T("Cylinder"))
{
}

wxIcon* CCylinder::GetIcon()
{
	if(m_icon == NULL){wxString exe_folder = wxGetApp().GetExeFolder();	m_icon = new wxIcon(exe_folder + _T("/icons/cyl.png"), wxBITMAP_TYPE_PNG);}
	return m_icon;
}

HeeksObj *CCylinder::MakeACopy(void)const
{
	return new CCylinder(*this);
}


// CCone

wxIcon* CCone::m_icon = NULL;

CCone::CCone(const gp_Ax2& pos, double r1, double r2, double height):m_pos(pos), m_r1(r1), m_r2(r2), m_height(height), CSolid(BRepPrimAPI_MakeCone(pos, r1, r2, height), _T("Cone"))
{
}

wxIcon* CCone::GetIcon()
{
	if(m_icon == NULL){wxString exe_folder = wxGetApp().GetExeFolder();	m_icon = new wxIcon(exe_folder + _T("/icons/cone.png"), wxBITMAP_TYPE_PNG);}
	return m_icon;
}

HeeksObj *CCone::MakeACopy(void)const
{
	return new CCone(*this);
}


// CSolid

wxIcon* CSolid::m_icon = NULL;

CSolid::CSolid(const TopoDS_Solid &solid, const wxChar* title, bool use_one_gl_list):CShape(solid, title, use_one_gl_list)
{
}

CSolid::~CSolid()
{
}

wxIcon* CSolid::GetIcon()
{
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + _T("/icons/solid.png"), wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}

HeeksObj *CSolid::MakeACopy(void)const
{
	return new CSolid(*this);
}
