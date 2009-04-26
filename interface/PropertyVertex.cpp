// PropertyVertex.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "PropertyVertex.h"

PropertyVertex::PropertyVertex(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object)):Property(){
	title = wxString(t);
	memcpy(m_x, initial_vt, 3*sizeof(double));
	m_callbackfunc = callbackfunc;
	m_callbackfuncidx = 0;
	m_object = object;
	has_index = false;
}

PropertyVertex::PropertyVertex(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object, int), int index){
	title = wxString(t);
	memcpy(m_x, initial_vt, 3*sizeof(double));
	m_callbackfunc = 0;
	m_callbackfuncidx = callbackfunc;
	m_object = object;
	has_index = true;

}

PropertyVertex::~PropertyVertex(){
}

const wxChar* PropertyVertex::GetShortString()const{
	return title.c_str();
}

Property *PropertyVertex::MakeACopy(void)const{
	PropertyVertex* new_object = new PropertyVertex(*this);
	return new_object;
}

void PropertyVertex::CallSetFunction() const{
	if(m_callbackfunc)(*m_callbackfunc)(m_x, m_object);
	if(m_callbackfuncidx)(*m_callbackfuncidx)(m_x,m_object,m_index);
}

