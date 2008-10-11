// PropertyCheck.cpp

#include "stdafx.h"

#include "PropertyCheck.h"

PropertyCheck::PropertyCheck(const wxChar* t, bool initial_value, HeeksObj* object,  void(*callbackfunc)(bool, HeeksObj*)):Property(){
	m_callbackfunc = callbackfunc;
	m_initial_value = initial_value;
	m_object = object;
	title = wxString(t);
}

PropertyCheck::~PropertyCheck(){
}

const wxChar* PropertyCheck::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyCheck::MakeACopy(void)const{
	PropertyCheck* new_object = new PropertyCheck(*this);
	return new_object;
}
