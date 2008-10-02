// PropertyChoice.cpp

#include "stdafx.h"

#include "PropertyChoice.h"

PropertyChoice::PropertyChoice(const wxChar* t, std::list< wxString > &choices, int initial_index, void(*callbackfunc)(int)):Property(){
	m_callbackfunc = callbackfunc;
	m_choices = choices;
	m_initial_index = initial_index;
	title = wxString(t);
}

PropertyChoice::~PropertyChoice(){
}

const wxChar* PropertyChoice::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyChoice::MakeACopy(void)const{
	PropertyChoice* new_object = new PropertyChoice(*this);
	return new_object;
}
