// Cone.cpp

#include "stdafx.h"
#include "Cone.h"
#include <BRepPrimAPI_MakeCone.hxx>

wxIcon* CCone::m_icon = NULL;

CCone::CCone(const gp_Ax2& pos, double r1, double r2, double height, const wxChar* title):m_pos(pos), m_r1(r1), m_r2(r2), m_height(height), CSolid(BRepPrimAPI_MakeCone(pos, r1, r2, height), title)
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

void CCone::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);
}

void CCone::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
}
