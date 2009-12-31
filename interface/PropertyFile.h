// PropertyFile.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined PropertyFile_HEADER
#define PropertyFile_HEADER

#include "PropertyString.h"

class PropertyFile:public PropertyString{
public:
	wxString m_title;

	PropertyFile(const wxChar* t, const wxChar* v, HeeksObj* object, void(*callbackfunc)(const wxChar*, HeeksObj*) = NULL);

	// Property's virtual functions
	int get_property_type(){return FilePropertyType;}
	Property *MakeACopy(void)const;
};

#endif
