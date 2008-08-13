// PropertyVertex.cpp

#include "stdafx.h"
#include "PropertyVertex.h"

PropertyVertex::PropertyVertex(const char *t, const gp_Pnt &initial_vt, void(*callbackfunc)(const gp_Pnt& vt)):Property(){
	title = std::string(t);
	m_vt = initial_vt;
	m_callbackfunc = callbackfunc;
}

PropertyVertex::~PropertyVertex(){
}

const char* PropertyVertex::GetShortString()const{
	return title.c_str();
}

Property *PropertyVertex::MakeACopy(void)const{
	PropertyVertex* new_object = new PropertyVertex(*this);
	return new_object;
}
