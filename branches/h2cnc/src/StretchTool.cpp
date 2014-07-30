// StretchTool.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "StretchTool.h"
#include "../interface/HeeksObj.h"

StretchTool::StretchTool(HeeksObj *object, const double *p, const double* shift, void* data){
	m_object = object;
	memcpy(m_pos, p, 3*sizeof(double));
	memcpy(m_shift, shift, 3*sizeof(double));
	m_undo_uses_add = false;
	m_data = data;
}

StretchTool::~StretchTool(void){
}

wxString stretch_function_string;

// Tool's virtual functions
const wxChar* StretchTool::GetTitle(){
	stretch_function_string.assign(wxString::Format(_T("Stretch %s"), m_object->GetShortStringOrTypeString()));
	return stretch_function_string.c_str();
}

void StretchTool::Run(){
	m_undo_uses_add = m_object->Stretch(m_pos, m_shift, m_data);
	for(int i = 0; i<3; i++)m_new_pos[i]= m_pos[i] + m_shift[i];
}

void StretchTool::RollBack(){
	if(!m_undo_uses_add){
		double unshift[3];
		for(int i = 0; i<3; i++){
			unshift[i] = -m_shift[i];
		}
		m_object->Stretch(m_new_pos, unshift, m_data);
	}
}
