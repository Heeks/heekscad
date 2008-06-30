// PropertyVertex.h

#pragma once

#include "../interface/Property.h"

class PropertyVertex:public Property{
private:
	std::string title;

public:
	gp_Pnt m_vt;
	void(*m_callbackfunc)(gp_Pnt& vt);
	
	PropertyVertex(const char *t, const gp_Pnt &initial_vt, void(*callbackfunc)(gp_Pnt& vt) = NULL);
	~PropertyVertex();

	// Property's virtual functions
	int get_property_type(){return VertexPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;

	// HeeksObj's virtual functions
	const char* GetShortString(void)const;
};

