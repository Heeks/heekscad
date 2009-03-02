// PropertyVertex.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "PropertyVertex.h"

PropertyVertex::PropertyVertex(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object)):Property(){
	title = wxString(t);
	memcpy(m_x, initial_vt, 3*sizeof(double));
	m_callbackfunc = callbackfunc;
	m_object = object;
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
