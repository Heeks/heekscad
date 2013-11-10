// PropertyChoice.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined PropertyChoice_HEADER
#define PropertyChoice_HEADER

#include "Property.h"

class PropertyChoice:public Property{
private:
	wxString title;

public:
	std::list< wxString > m_choices;
	void (*m_callbackfunc)(int choice, HeeksObj*, bool from_undo_redo); // 0 is the first
	int m_initial_index;

	PropertyChoice(const wxChar* t, std::list< wxString > &choices, int initial_index, HeeksObj* object,  void(*callbackfunc)(int, HeeksObj*, bool) = NULL, void(*selectcallback)(HeeksObj*) = NULL);
	~PropertyChoice();

	// Property's virtual functions
	int get_property_type(){return ChoicePropertyType;}
	bool property_editable()const{return m_callbackfunc != NULL;}
	Property *MakeACopy(void)const;
	const wxChar* GetShortString(void)const;
};

#endif
