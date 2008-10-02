// PropertyString.cpp

#include "stdafx.h"

#include "PropertyString.h"

PropertyString::PropertyString(const wxChar* t, const wxChar* v, void(*callbackfunc)(const wxChar*)):Property() , m_callbackfunc(callbackfunc){
	m_title = wxString(t);
	m_initial_value = wxString(v);
}

const wxChar* PropertyString::GetShortString(void)const{
	return m_title.c_str();
}

Property *PropertyString::MakeACopy(void)const{
	PropertyString* new_object = new PropertyString(*this);
	return new_object;
}
