// PropertyList.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyList.h"

PropertyList::PropertyList(const wxChar* t, void(*selectcallback)(HeeksObj*)):Property(NULL, selectcallback){
	title = wxString(t);
}

PropertyList::~PropertyList(){
	std::list< Property* >::iterator It;
	for(It = m_list.begin(); It != m_list.end(); It++)
	{
		Property* property = *It;
		delete property;
	}
}

const wxChar* PropertyList::GetShortString(void)const{
	return title.c_str();
}

Property *PropertyList::MakeACopy(void)const{
	PropertyList* new_object = new PropertyList(*this);
	return new_object;
}
