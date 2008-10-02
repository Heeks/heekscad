// PropertyVertex.h

#pragma once

#include "../interface/Property.h"

class PropertyVertex:public Property{
private:
	wxString title;

public:
	gp_Pnt m_vt;
	void(*m_callbackfunc)(const gp_Pnt& vt);
	
	PropertyVertex(const wxChar *t, const gp_Pnt &initial_vt, void(*callbackfunc)(const gp_Pnt& vt) = NULL);
	~PropertyVertex();

	// Property's virtual functions
	int get_property_type(){return VertexPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_vt);}

	// HeeksObj's virtual functions
	const wxChar* GetShortString(void)const;
};

