// Solid.cpp
#include "stdafx.h"
#include "Solid.h"
#include "icons/solid.xpm"

wxIcon* CSolid::m_icon = NULL;

CSolid::CSolid(const TopoDS_Solid &solid, const char* title, bool use_one_gl_list):CShape(solid, title, use_one_gl_list)
{
}

CSolid::~CSolid()
{
}

wxIcon* CSolid::GetIcon()
{
	if(m_icon == NULL)m_icon = new wxIcon(solid_xpm);
	return m_icon;
}

HeeksObj *CSolid::MakeACopy(void)const
{
	return new CSolid(*this);
}