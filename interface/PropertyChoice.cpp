// PropertyChoice.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyChoice.h"

PropertyChoice::PropertyChoice(const wxChar* t, std::list< wxString > &choices, int initial_index, HeeksObj* object, void(*callbackfunc)(int, HeeksObj*), void(*selectcallback)(HeeksObj*)):Property(object, selectcallback){
	m_callbackfunc = callbackfunc;
	m_choices = choices;
	m_initial_index = initial_index;
	title = wxString(t);
}

PropertyChoice::~PropertyChoice(){
}

const wxChar* PropertyChoice::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyChoice::MakeACopy(void)const{
	PropertyChoice* new_object = new PropertyChoice(*this);
	return new_object;
}
