// PropertyColor.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyColor.h"

PropertyColor::PropertyColor(const wxChar* t, HeeksColor initial_value, HeeksObj* object, void(*callbackfunc)(HeeksColor, HeeksObj*), void(*selectcallback)(HeeksObj*)):Property(object, selectcallback){
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
