// PropertyTrsf.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "PropertyTrsf.h"

PropertyTrsf::PropertyTrsf(const wxChar *t, const gp_Trsf &initial_trsf, HeeksObj* object, void(*callbackfunc)(const gp_Trsf& trsf, HeeksObj* m_object), void(*selectcallback)(HeeksObj*)):Property(object, selectcallback){
	title = wxString(t);
	m_trsf = initial_trsf;
	m_callbackfunc = callbackfunc;
}

PropertyTrsf::~PropertyTrsf(){
}

const wxChar* PropertyTrsf::GetShortString()const{
	return title.c_str();
}

Property *PropertyTrsf::MakeACopy(void)const{
	PropertyTrsf* new_object = new PropertyTrsf(*this);
	return new_object;
}
