// PropertyChoice.h

#if !defined PropertyChoice_HEADER
#define PropertyChoice_HEADER

#include "Property.h"

class PropertyChoice:public Property{
private:
	std::string title;

public:
	std::list< std::string > m_choices;
	void (*m_callbackfunc)(int choice); // 0 is the first
	int m_initial_index;

	PropertyChoice(const char* t, std::list< std::string > &choices, int initial_index,  void(*callbackfunc)(int) = NULL);
	~PropertyChoice();

	// Property's virtual functions
	int get_property_type(){return ChoicePropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_initial_index);}

	// HeeksObj's virtual functions
	const char* GetShortString(void)const;
};

#endif
