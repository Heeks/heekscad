// Property.h

// Base class for all Properties

#if !defined Property_HEADER
#define Property_HEADER

enum{
	InvalidPropertyType,
	StringPropertyType,
	DoublePropertyType,
	IntPropertyType,
	VertexPropertyType,
	ChoicePropertyType,
	ColorPropertyType,
	CheckPropertyType,
	ListOfPropertyType
};

#include <vector>

class Property{
public:
	Property(){}
	virtual ~Property(){}

	virtual int get_property_type(){return InvalidPropertyType;}
	virtual bool property_editable()const = 0;
	virtual Property *MakeACopy(void)const = 0;
	virtual const char* GetShortString(void)const{return "Unknown Property";}
};

#endif
