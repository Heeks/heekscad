// PropertyVertex.h

#pragma once

#include "../interface/Property.h"

class PropertyVertex:public Property{
private:
	wxString title;

public:
	double m_x[3];
	HeeksObj* m_object;
	void(*m_callbackfunc)(const double*, HeeksObj*);
	
	PropertyVertex(const wxChar *t, const double *initial_vt, HeeksObj* object, void(*callbackfunc)(const double* vt, HeeksObj* m_object) = NULL);
	~PropertyVertex();

	// Property's virtual functions
	int get_property_type(){return VertexPropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	void CallSetFunction()const{ if(m_callbackfunc)(*m_callbackfunc)(m_x, m_object);}
	const wxChar* GetShortString(void)const;
};

