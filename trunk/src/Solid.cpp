// Solid.cpp
#include "stdafx.h"
#include "Solid.h"

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