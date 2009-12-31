// PropertyFile.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "PropertyFile.h"

PropertyFile::PropertyFile(const wxChar* t, const wxChar* v, HeeksObj* object, void(*callbackfunc)(const wxChar*, HeeksObj*)):PropertyString(t, v, object, callbackfunc)
{
}

Property *PropertyFile::MakeACopy(void)const{
	PropertyFile* new_object = new PropertyFile(*this);
	return new_object;
}
