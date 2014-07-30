// PropertyVertex.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Property.h"

class PropertyVertex:public Property{
protected:
	wxString title;

public:
	double m_x[3];
	int m_index;
	bool has_index;
	bool m_affected_by_view_units;
	void(*m_callbackfunc)(const double*, HeeksObj*);
	void(*m_callbackfuncidx)(const double*, HeeksObj*,int);
	
	PropertyVertex(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object) = NULL, void(*selectcallback)(HeeksObj*) = NULL);
	PropertyVertex(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object, int), int index, void(*selectcallback)(HeeksObj*) = NULL);
	~PropertyVertex();

	// Property's virtual functions
	int get_property_type(){return VertexPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL || m_callbackfuncidx != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const;
	const wxChar* GetShortString(void)const;

	virtual bool xyOnly()const{return false;}
};

class PropertyVertex2d: public PropertyVertex{
public:
	PropertyVertex2d(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object) = NULL, void(*selectcallback)(HeeksObj*) = NULL):PropertyVertex(t, initial_vt, object, callbackfunc, selectcallback){}

	// PropertyVertex's virtual functions
	bool xyOnly()const{return true;}
};

class PropertyVector: public PropertyVertex{
	// like a PropertyVertex, but isn't affected by view units
public:
	PropertyVector(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object) = NULL) : PropertyVertex(t, initial_vt, object, callbackfunc){m_affected_by_view_units = false;}
	~PropertyVector(){}

	Property *MakeACopy(void)const;
};
