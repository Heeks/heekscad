// PropertyColor.h

#pragma once

#include "Property.h"
#include "HeeksColor.h"

class PropertyColor:public Property{
private:
	std::string title;

public:
	HeeksColor m_initial_value;
	void(*m_callbackfunc)(HeeksColor);

	PropertyColor(const char* t, HeeksColor initial_value, void(*callbackfunc)(HeeksColor value) = NULL);
	~PropertyColor();

	// Property's virtual functions
	int get_property_type(){return ColorPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;

	// HeeksObj's virtual functions
	const char* GetShortString(void)const;
};
