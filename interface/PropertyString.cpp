// PropertyString.cpp

#include "stdafx.h"

#include "PropertyString.h"

PropertyString::PropertyString(const char* t, const char* v, void(*callbackfunc)(const char*)):Property() , m_callbackfunc(callbackfunc){
	m_title = std::string(t);
	m_initial_value = std::string(v);
}

const char* PropertyString::GetShortString(void)const{
	return m_title.c_str();
}

Property *PropertyString::MakeACopy(void)const{
	PropertyString* new_object = new PropertyString(*this);
	return new_object;
}
