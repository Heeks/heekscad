// Cylinder.cpp

#include "stdafx.h"
#include "Cylinder.h"
#include <BRepPrimAPI_MakeCylinder.hxx>

wxIcon* CCylinder::m_icon = NULL;

CCylinder::CCylinder(const gp_Ax2& pos, double radius, double height, const wxChar* title):m_pos(pos), m_radius(radius), m_height(height), CSolid(BRepPrimAPI_MakeCylinder(pos, radius, height), title)
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

void CCylinder::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);
}

void CCylinder::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
}
