// PropertyDouble.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined PropertyDouble_HEADER
#define PropertyDouble_HEADER

#include "Property.h"

class PropertyDouble:public Property{
private:
	wxString title;

public:
	double m_initial_value;
	void(*m_callbackfunc)(double, HeeksObj*);
	void(*m_callbackfuncidx)(double, HeeksObj*,int);
	int m_index;
	bool has_index;

	PropertyDouble(const wxChar* t, double initial_value, HeeksObj* object, void(*callbackfunc)(double, HeeksObj*) = NULL, void(*selectcallback)(HeeksObj*) = NULL);
	PropertyDouble(const wxChar* t, double initial_value, HeeksObj* object, void(*callbackfunc)(double, HeeksObj*,int), int index, void(*selectcallback)(HeeksObj*) = NULL);
	~PropertyDouble();

	// Property's virtual functions
	int get_property_type(){return DoublePropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL || m_callbackfuncidx != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const;
	const wxChar* GetShortString(void)const;
};

#endif
