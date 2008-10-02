// PropertyColor.h

#pragma once

#include "Property.h"
#include "HeeksColor.h"

class PropertyColor:public Property{
private:
	wxString title;

public:
	HeeksColor m_initial_value;
	void(*m_callbackfunc)(HeeksColor);

	PropertyColor(const wxChar* t, HeeksColor initial_value, void(*callbackfunc)(HeeksColor value) = NULL);
	~PropertyColor();

	// Property's virtual functions
	int get_property_type(){return ColorPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_initial_value);}

	// HeeksObj's virtual functions
	const wxChar* GetShortString(void)const;
};
