// PropertyChoice.h

#if !defined PropertyChoice_HEADER
#define PropertyChoice_HEADER

#include "Property.h"

class PropertyChoice:public Property{
private:
	wxString title;

public:
	std::list< wxString > m_choices;
	void (*m_callbackfunc)(int choice); // 0 is the first
	int m_initial_index;

	PropertyChoice(const wxChar* t, std::list< wxString > &choices, int initial_index,  void(*callbackfunc)(int) = NULL);
	~PropertyChoice();

	// Property's virtual functions
	int get_property_type(){return ChoicePropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_initial_index);}

	// HeeksObj's virtual functions
	const wxChar* GetShortString(void)const;
};

#endif
