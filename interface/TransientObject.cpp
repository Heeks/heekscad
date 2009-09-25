// TransientObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "TransientObject.h"

TransientObject::TransientObject(HeeksObj* obj)
{
	m_object = obj;
#ifdef HEEKSCAD
	wxGetApp().WentTransient(obj,this);
#else
	heeksCAD->WentTransient(obj, this);
#endif
}

HeeksObj* TransientObject::MakeACopy()const
{
	TransientObject *tobj = new TransientObject(*this);
#ifdef HEEKSCAD
	wxGetApp().WentTransient(m_object,tobj);
#else
	heeksCAD->WentTransient(m_object,tobj);
#endif
	return tobj;
}
