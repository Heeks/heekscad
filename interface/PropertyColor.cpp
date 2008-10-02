// PropertyColor.cpp

#include "stdafx.h"

#include "PropertyColor.h"

PropertyColor::PropertyColor(const wxChar* t, HeeksColor initial_value, void(*callbackfunc)(HeeksColor)):Property(){
	m_initial_value = initial_value;
	m_callbackfunc = callbackfunc;
	title = wxString(t);
}

PropertyColor::~PropertyColor(){
}

const wxChar* PropertyColor::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyColor::MakeACopy(void)const{
	PropertyColor* new_object = new PropertyColor(*this);
	return new_object;
}
