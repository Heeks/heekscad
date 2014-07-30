// PropertyTrsf.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Property.h"

class PropertyTrsf:public Property{
private:
	wxString title;

public:
	gp_Trsf m_trsf;
	void(*m_callbackfunc)(const gp_Trsf&, HeeksObj*);
	
	PropertyTrsf(const wxChar *t, const gp_Trsf &initial_trsf, HeeksObj* object, void(*callbackfunc)(const gp_Trsf& trsf, HeeksObj* m_object) = NULL, void(*selectcallback)(HeeksObj*) = NULL);
	~PropertyTrsf();

	// Property's virtual functions
	int get_property_type(){return TrsfPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_trsf, m_object);}
	const wxChar* GetShortString(void)const;
};

