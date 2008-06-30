// PropertyCheck.h

#if !defined PropertyCheck_HEADER
#define PropertyCheck_HEADER

#include "Property.h"

class PropertyCheck:public Property{
private:
	std::string title;

public:
	void (*m_callbackfunc)(bool); // onoff
	bool m_initial_value;

	PropertyCheck(const char* t, bool initial_value,  void(*callbackfunc)(bool) = NULL);
	~PropertyCheck();

	// Property's virtual functions
	int get_property_type(){return CheckPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	const char* GetShortString(void)const;
};

#endif
