// PropertyDouble.cpp

#include "stdafx.h"

#include "PropertyDouble.h"

PropertyDouble::PropertyDouble(const wxChar* t, double initial_value, void(*callbackfunc)(double)):Property(){
	m_initial_value = initial_value;
	m_callbackfunc = callbackfunc;
	title = wxString(t);
}

PropertyDouble::~PropertyDouble(){
}

const wxChar* PropertyDouble::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyDouble::MakeACopy(void)const{
	PropertyDouble* new_object = new PropertyDouble(*this);
	return new_object;
}
