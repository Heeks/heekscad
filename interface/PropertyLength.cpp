// PropertyLength.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyLength.h"

#ifdef HEEKSCAD
	#define VIEW_UNITS (wxGetApp().m_view_units)
#else
	#define VIEW_UNITS (heeksCAD->GetViewUnits())
#endif

PropertyLength::PropertyLength(const wxChar* t, double initial_value, HeeksObj* object, void(*callbackfunc)(double, HeeksObj*), void(*selectcallback)(HeeksObj*)):PropertyDouble(t, initial_value/VIEW_UNITS, object, callbackfunc, selectcallback){
}

Property *PropertyLength::MakeACopy(void)const{
	PropertyLength* new_object = new PropertyLength(*this);
	return new_object;
}
void PropertyLength::CallSetFunction()const{
	if(m_callbackfunc)(*m_callbackfunc)(m_initial_value * VIEW_UNITS, m_object);
}

