// Property.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// Base class for all Properties

#if !defined Property_HEADER
#define Property_HEADER

enum{
	InvalidPropertyType,
	StringPropertyType,
	DoublePropertyType,
	LengthPropertyType,
	IntPropertyType,
	VertexPropertyType,
	ChoicePropertyType,
	ColorPropertyType,
	CheckPropertyType,
	ListOfPropertyType,
	TrsfPropertyType,
	FilePropertyType
};

#include <vector>

class Property{
public:
	bool m_highlighted;
	HeeksObj* m_object;

	void(*m_selectcallback)(HeeksObj*);

	Property(HeeksObj* object, void(*selectcallback)(HeeksObj*) = NULL, bool highlighted = false):m_highlighted(highlighted), m_object(object), m_selectcallback(selectcallback) {}
	virtual ~Property(){}

	virtual int get_property_type(){return InvalidPropertyType;}
	virtual bool property_editable()const = 0;
	virtual Property *MakeACopy(void)const = 0;
	virtual void CallSetFunction()const = 0;
	virtual const wxChar* GetShortString(void)const{return _("Unknown Property");}
};

#endif
