// PropertyVertex.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Property.h"

class PropertyVertex:public Property{
private:
	wxString title;

public:
	double m_x[3];
	HeeksObj* m_object;
	int m_index;
	bool has_index;
	void(*m_callbackfunc)(const double*, HeeksObj*);
	void(*m_callbackfuncidx)(const double*, HeeksObj*,int);
	
	PropertyVertex(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object) = NULL);
	PropertyVertex(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object, int), int index);
	~PropertyVertex();

	// Property's virtual functions
	int get_property_type(){return VertexPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL || m_callbackfuncidx != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const;
	const wxChar* GetShortString(void)const;
};

