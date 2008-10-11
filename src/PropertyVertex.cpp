// PropertyVertex.cpp

#include "stdafx.h"
#include "PropertyVertex.h"

PropertyVertex::PropertyVertex(const wxChar *t, const gp_Pnt &initial_vt, HeeksObj* object, void(*callbackfunc)(const gp_Pnt& vt, HeeksObj* m_object)):Property(){
	title = wxString(t);
	m_vt = initial_vt;
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
