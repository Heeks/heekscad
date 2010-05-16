// PropertyInt.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyInt.h"

PropertyInt::PropertyInt(const wxChar* t, int initial_value, HeeksObj* object, void(*callbackfunc)(int, HeeksObj*), void(*selectcallback)(HeeksObj*)):Property(object, selectcallback){
	m_initial_value = initial_value;
	m_callbackfunc = callbackfunc;
	title = wxString(t);
}

PropertyInt::~PropertyInt(){
}

const wxChar* PropertyInt::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyInt::MakeACopy(void)const{
	PropertyInt* new_object = new PropertyInt(*this);
	return new_object;
}
