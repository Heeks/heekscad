// PropertyInt.cpp

#include "stdafx.h"

#include "PropertyInt.h"

PropertyInt::PropertyInt(const char* t, int initial_value, void(*callbackfunc)(int)):Property(){
	m_initial_value = initial_value;
	m_callbackfunc = callbackfunc;
	title = std::string(t);
}

PropertyInt::~PropertyInt(){
}

const char* PropertyInt::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyInt::MakeACopy(void)const{
	PropertyInt* new_object = new PropertyInt(*this);
	return new_object;
}
