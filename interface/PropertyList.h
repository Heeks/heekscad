// PropertyList.h

#if !defined PropertyList_HEADER
#define PropertyList_HEADER

#include "Property.h"

class PropertyList:public Property{
private:
	std::string title;

public:
	std::list< Property* > m_list;

	PropertyList(const char* t);
	~PropertyList();

	// Property's virtual functions
	int get_property_type(){return ListOfPropertyType;}
	bool property_editable()const{return false;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ for(std::list< Property* >::const_iterator It = m_list.begin(); It != m_list.end(); It++)(*It)->CallSetFunction();}

	// HeeksObj's virtual functions
	const char* GetShortString(void)const;
};

#endif
