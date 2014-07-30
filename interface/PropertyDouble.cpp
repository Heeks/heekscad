// PropertyDouble.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyDouble.h"

PropertyDouble::PropertyDouble(const wxChar* t, double initial_value, HeeksObj* object, void(*callbackfunc)(double, HeeksObj*), void(*selectcallback)(HeeksObj*)):Property(object, selectcallback){
	m_initial_value = initial_value;
	m_callbackfunc = callbackfunc;
	m_callbackfuncidx = 0;
	title = wxString(t);
	has_index = false;
}

PropertyDouble::PropertyDouble(const wxChar* t, double initial_value, HeeksObj* object, void(*callbackfunc)(double, HeeksObj*, int), int index, void(*selectcallback)(HeeksObj*)):Property(object, selectcallback)
{
	m_initial_value = initial_value;
	m_callbackfuncidx = callbackfunc;
	m_callbackfunc = 0;
	m_object = object;
	title = wxString(t);
	has_index = true;
	m_index = index;
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

void PropertyDouble::CallSetFunction() const
{ 
	if(m_callbackfunc)(*m_callbackfunc)(m_initial_value, m_object);
	if(m_callbackfuncidx)(*m_callbackfuncidx)(m_initial_value, m_object,m_index);
}

