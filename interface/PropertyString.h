// PropertyString.h

#if !defined PropertyString_HEADER
#define PropertyString_HEADER

#include "Property.h"

class PropertyString:public Property{
private:
	std::string m_title;

public:
	std::string m_initial_value;
	void(*m_callbackfunc)(const char*);

	PropertyString(const char* t, const char* v, void(*callbackfunc)(const char*) = NULL);

	// Property's virtual functions
	int get_property_type(){return StringPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;

	// HeeksObj's virtual functions
	const char* GetShortString(void)const;
};

#endif
