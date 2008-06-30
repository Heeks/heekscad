// StretchTool.cpp
#include "stdafx.h"
#include "StretchTool.h"
#include "../interface/HeeksObj.h"

StretchTool::StretchTool(HeeksObj *object, const double *p, const double* shift){
	m_object = object;
	memcpy(m_pos, p, 3*sizeof(double));
	memcpy(m_shift, shift, 3*sizeof(double));
}

StretchTool::~StretchTool(void){
}

std::string stretch_function_string;

// Tool's virtual functions
const char* StretchTool::GetTitle(){
	char str[1024];
	sprintf(str, "Stretch %s", m_object->GetShortStringOrTypeString());
	stretch_function_string.assign(str);
	return stretch_function_string.c_str();
}

void StretchTool::Run(){
	m_object->Stretch(m_pos, m_shift, m_new_pos);
}

void StretchTool::RollBack(){
	double unshift[3];
	for(int i = 0; i<3; i++){
		unshift[i] = -m_shift[i];
	}
	double dummy_pos[3];
	m_object->Stretch(m_new_pos, unshift, dummy_pos);
}
