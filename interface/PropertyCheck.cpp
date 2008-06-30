// PropertyCheck.cpp

#include "stdafx.h"

#include "PropertyCheck.h"

PropertyCheck::PropertyCheck(const char* t, bool initial_value,  void(*callbackfunc)(bool)):Property(){
	m_callbackfunc = callbackfunc;
	m_initial_value = initial_value;
	title = std::string(t);
}

PropertyCheck::~PropertyCheck(){
}

const char* PropertyCheck::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyCheck::MakeACopy(void)const{
	PropertyCheck* new_object = new PropertyCheck(*this);
	return new_object;
}
