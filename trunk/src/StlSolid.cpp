// StlSolid.cpp
#include "stdafx.h"
#include "StlSolid.h"
#include "icons/stlsolid.xpm"

wxIcon* CStlSolid::m_icon = NULL;

CStlSolid::CStlSolid(){
}

CStlSolid::~CStlSolid(){
}

wxIcon* CStlSolid::GetIcon(){
	if(m_icon == NULL)m_icon = new wxIcon(stlsolid_xpm);
	return m_icon;
}
