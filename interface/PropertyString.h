// PropertyString.h

#if !defined PropertyString_HEADER
#define PropertyString_HEADER

#include "Property.h"

class PropertyString:public Property{
private:
	wxString m_title;

public:
	wxString m_initial_value;
	void(*m_callbackfunc)(const wxChar*);

	PropertyString(const wxChar* t, const wxChar* v, void(*callbackfunc)(const wxChar*) = NULL);

	// Property's virtual functions
	int get_property_type(){return StringPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_initial_value.c_str());}

	// HeeksObj's virtual functions
	const wxChar* GetShortString(void)const;
};

#endif
