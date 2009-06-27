// EndedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "EndedObject.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyVertex.h"
#include "../tinyxml/tinyxml.h"
#include "HPoint.h"
#include "MarkedList.h"

EndedObject::EndedObject(const HeeksColor* color){
	A = new HPoint(gp_Pnt(),color);
	B = new HPoint(gp_Pnt(),color);
	A->m_draw_unselected = false;
	B->m_draw_unselected = false;
	Add(A,NULL);
	Add(B,NULL);
}

EndedObject::~EndedObject(){
}

const EndedObject& EndedObject::operator=(const EndedObject &b){
	ObjList::operator=(b);
	A = new HPoint(*b.A);
	B = new HPoint(*b.B);
	A->m_draw_unselected = false;
	B->m_draw_unselected = false;
	Add(A,NULL);
	Add(B,NULL);
	return *this;
}

void EndedObject::LoadToDoubles()
{
	A->LoadToDoubles();
	B->LoadToDoubles();
}

void EndedObject::LoadFromDoubles()
{
	A->LoadFromDoubles();
	B->LoadFromDoubles();
}

bool EndedObject::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A->m_p.Transform(mat);
	B->m_p.Transform(mat);
	return false;
}

bool EndedObject::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(data == &A){
		A->m_p = vp.XYZ() + vshift.XYZ();
	}
	else if(data == &B){
		B->m_p = vp.XYZ() + vshift.XYZ();
	}
	return false;
}

void EndedObject::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	list->push_back(GripData(GripperTypeStretch,A->m_p.X(),A->m_p.Y(),A->m_p.Z(),&A));
	list->push_back(GripData(GripperTypeStretch,B->m_p.X(),B->m_p.Y(),B->m_p.Z(),&B));
}

bool EndedObject::GetStartPoint(double* pos)
{
	extract(A->m_p, pos);
	return true;
}

bool EndedObject::GetEndPoint(double* pos)
{
	extract(B->m_p, pos);
	return true;
}

void EndedObject::glCommands(bool select, bool marked, bool no_color)
{
	HeeksObj::glCommands(select, marked, no_color);
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(object->OnVisibleLayer() && object->m_visible)
		{
			if(select)glPushName((unsigned long)object);
#ifdef HEEKSCAD
			(*It)->glCommands(select, marked || wxGetApp().m_marked_list->ObjectMarked(object), no_color);
#else
			(*It)->glCommands(select, marked || heeksCAD->ObjectMarked(object), no_color);
#endif
			if(select)glPopName();
		}
	}
}

void EndedObject::WriteBaseXML(TiXmlElement *element)
{
	HeeksObj::WriteBaseXML(element);
}