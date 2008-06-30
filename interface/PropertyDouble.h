// PropertyDouble.h

#if !defined PropertyDouble_HEADER
#define PropertyDouble_HEADER

#include "Property.h"

class PropertyDouble:public Property{
private:
	std::string title;

public:
	double m_initial_value;
	void(*m_callbackfunc)(double);

	PropertyDouble(const char* t, double initial_value, void(*callbackfunc)(double) = NULL);
	~PropertyDouble();

	// Property's virtual functions
	int get_property_type(){return DoublePropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;

	// HeeksObj's virtual functions
	const char* GetShortString(void)const;
};

#endif
