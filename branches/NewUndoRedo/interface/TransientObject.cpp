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

HeeksObj* TransientObject::MakeACopy()const
{
	TransientObject *tobj = new TransientObject(*this);
	wxGetApp().WentTransient(m_object,tobj);
	return tobj;
}