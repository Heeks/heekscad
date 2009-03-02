// PropertyString.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyString.h"

PropertyString::PropertyString(const wxChar* t, const wxChar* v, HeeksObj* object, void(*callbackfunc)(const wxChar*, HeeksObj*)):Property() , m_callbackfunc(callbackfunc){
	m_title = wxString(t);
	m_object = object;
	m_initial_value = wxString(v);
}

const wxChar* PropertyString::GetShortString(void)const{
	return m_title.c_str();
}

Property *PropertyString::MakeACopy(void)const{
	PropertyString* new_object = new PropertyString(*this);
	return new_object;
}
