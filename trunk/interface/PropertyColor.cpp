// PropertyColor.cpp

#include "stdafx.h"

#include "PropertyColor.h"

PropertyColor::PropertyColor(const char* t, HeeksColor initial_value, void(*callbackfunc)(HeeksColor)):Property(){
	m_initial_value = initial_value;
	m_callbackfunc = callbackfunc;
	title = std::string(t);
}

PropertyColor::~PropertyColor(){
}

const char* PropertyColor::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyColor::MakeACopy(void)const{
	PropertyColor* new_object = new PropertyColor(*this);
	return new_object;
}
