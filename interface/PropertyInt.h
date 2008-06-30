// PropertyInt.h

#if !defined PropertyInt_HEADER
#define PropertyInt_HEADER

#include "Property.h"

class PropertyInt:public Property{
private:
	std::string title;

public:
	int m_initial_value;
	void(*m_callbackfunc)(int);

	PropertyInt(const char* t, int initial_value, void(*callbackfunc)(int) = NULL);
	~PropertyInt();

	// Property's virtual functions
	int get_property_type(){return IntPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;

	// HeeksObj's virtual functions
	const char* GetShortString(void)const;
};

#endif
