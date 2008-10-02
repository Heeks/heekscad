// TransformTool.cpp
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
	wxChar str[1024];
	wsprintf(str, _T("Transform %s"), object->GetShortStringOrTypeString());
	global_string.assign(str);
	return global_string.c_str();
}

void TransformTool::Run(){
	object->ModifyByMatrix(modify_matrix);
}

void TransformTool::RollBack(){
	object->ModifyByMatrix(revert_matrix);
}

TransformObjectsTool::TransformObjectsTool(const std::list<HeeksObj*> &list, const gp_Trsf &t, const gp_Trsf &i){
	m_list = &list;
	extract(t, modify_matrix);
	extract(i, revert_matrix);
}

TransformObjectsTool::~TransformObjectsTool(void){
}

// Tool's virtual functions
const wxChar* TransformObjectsTool::GetTitle(){
	return _T("Transform Objects");
}

void TransformObjectsTool::Run(){
	std::list<HeeksObj*>::const_iterator It;
	for(It = m_list->begin(); It != m_list->end(); It++){
		HeeksObj* object = *It;
		object->ModifyByMatrix(modify_matrix);
	}

	wxGetApp().WereModified(*m_list);
}

void TransformObjectsTool::RollBack(){
	std::list<HeeksObj*>::const_iterator It;
	for(It = m_list->begin(); It != m_list->end(); It++){
		HeeksObj* object = *It;
		object->ModifyByMatrix(revert_matrix);
	}

	wxGetApp().WereModified(*m_list);
}
