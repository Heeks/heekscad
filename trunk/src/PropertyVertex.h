// PropertyVertex.h

#pragma once

#include "../interface/Property.h"

class PropertyVertex:public Property{
private:
	wxString title;

public:
	gp_Pnt m_vt;
	HeeksObj* m_object;
	void(*m_callbackfunc)(const gp_Pnt&, HeeksObj*);
	
	PropertyVertex(const wxChar *t, const gp_Pnt &initial_vt, HeeksObj* object, void(*callbackfunc)(const gp_Pnt& vt, HeeksObj* m_object) = NULL);
	~PropertyVertex();

	// Property's virtual functions
	int get_property_type(){return VertexPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_vt, m_object);}
	const wxChar* GetShortString(void)const;
};

