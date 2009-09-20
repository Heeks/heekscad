// ObjList.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "TransientObject.h"

TransientObject::TransientObject(HeeksObj* obj)
{
	m_object = obj;
	wxGetApp().WentTransient(obj,this);
}

//Transient objects can never be copied
HeeksObj* TransientObject::MakeACopy()const
{
	return NULL;
}