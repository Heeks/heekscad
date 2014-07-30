// TransformTool.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "TransformTool.h"
#include "../interface/HeeksObj.h"

TransformTool::TransformTool(HeeksObj *o, const gp_Trsf &t, const gp_Trsf &i){
	object = o;
	extract(t, modify_matrix);
	extract(i, revert_matrix);
}

TransformTool::~TransformTool(void){
}

wxString global_string;

// Tool's virtual functions
const wxChar* TransformTool::GetTitle(){
	global_string.assign(wxString::Format(_T("%s %s"), _("Transform"), object->GetShortStringOrTypeString()));
	return global_string.c_str();
}

void TransformTool::Run(){
	object->ModifyByMatrix(modify_matrix);
}

void TransformTool::RollBack(){
	object->ModifyByMatrix(revert_matrix);
}

TransformObjectsTool::TransformObjectsTool(const std::list<HeeksObj*> &list, const gp_Trsf &t, const gp_Trsf &i){
	m_list = list;
	extract(t, modify_matrix);
	extract(i, revert_matrix);
}

TransformObjectsTool::~TransformObjectsTool(void){
}

// Tool's virtual functions
const wxChar* TransformObjectsTool::GetTitle(){
	return _("Transform Objects");
}

void TransformObjectsTool::Run(){
	std::list<HeeksObj*>::iterator It;
	for(It = m_list.begin(); It != m_list.end(); It++){
		HeeksObj* object = *It;
		object->ModifyByMatrix(modify_matrix);
	}

	wxGetApp().Changed();
}

void TransformObjectsTool::RollBack(){
	std::list<HeeksObj*>::const_iterator It;
	for(It = m_list.begin(); It != m_list.end(); It++){
		HeeksObj* object = *It;
		object->ModifyByMatrix(revert_matrix);
	}

	wxGetApp().Changed();
}
