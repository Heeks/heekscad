// PropertyDouble.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyDouble.h"

PropertyDouble::PropertyDouble(const wxChar* t, double initial_value, HeeksObj* object, void(*callbackfunc)(double, HeeksObj*)):Property(){
	m_initial_value = initial_value;
	m_callbackfunc = callbackfunc;
	m_object = object;
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
