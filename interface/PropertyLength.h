// PropertyLength.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined PropertyLength_HEADER
#define PropertyLength_HEADER

#include "PropertyDouble.h"

class PropertyLength:public PropertyDouble{
public:
	PropertyLength(const wxChar* t, double initial_value, HeeksObj* object, void(*callbackfunc)(double, HeeksObj*) = NULL, void(*selectcallback)(HeeksObj*) = NULL);

	// Property's virtual functions
	int get_property_type(){return LengthPropertyType;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const;
};

#endif
