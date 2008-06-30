// StlSolid.cpp
#include "stdafx.h"
#include "StlSolid.h"

wxIcon* CStlSolid::m_icon = NULL;

CStlSolid::CStlSolid(){
}

CStlSolid::~CStlSolid(){
}

wxIcon* CStlSolid::GetIcon(){
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + "/icons/stlsolid.png", wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}
