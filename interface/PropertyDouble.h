// PropertyDouble.h

#if !defined PropertyDouble_HEADER
#define PropertyDouble_HEADER

#include "Property.h"

class PropertyDouble:public Property{
private:
	wxString title;

public:
	double m_initial_value;
	void(*m_callbackfunc)(double);

	PropertyDouble(const wxChar* t, double initial_value, void(*callbackfunc)(double) = NULL);
	~PropertyDouble();

	// Property's virtual functions
	int get_property_type(){return DoublePropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_initial_value);}

	// HeeksObj's virtual functions
	const wxChar* GetShortString(void)const;
};

#endif
