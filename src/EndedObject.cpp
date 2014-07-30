// EndedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "EndedObject.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyVertex.h"
#include "HPoint.h"
#include "MarkedList.h"

EndedObject::EndedObject(const HeeksColor* color){
	A = new HPoint(gp_Pnt(),color);
	B = new HPoint(gp_Pnt(),color);
#ifdef MULTIPLE_OWNERS
	A->m_draw_unselected = false;
	B->m_draw_unselected = false;
	A->SetSkipForUndo(true);
	B->SetSkipForUndo(true);
	Add(A,NULL);
	Add(B,NULL);
#else
	A->SetSkipForUndo(true);
	B->SetSkipForUndo(true);
#endif
}

#ifndef MULTIPLE_OWNERS
EndedObject::EndedObject(const EndedObject& e)
{
	A = new HPoint(gp_Pnt(), e.A->GetColor());
	B = new HPoint(gp_Pnt(), e.B->GetColor());
	A->SetSkipForUndo(true);
	B->SetSkipForUndo(true);
	operator=(e);
}
#endif

EndedObject::~EndedObject(){
}

const EndedObject& EndedObject::operator=(const EndedObject &b){
#ifdef MULTIPLE_OWNERS
	ObjList::operator=(b);
	std::list<HeeksObj*>::iterator it = m_objects.begin();
	A = (HPoint*)(*it++);
	B = (HPoint*)(*it);
	A->SetSkipForUndo(true);
	B->SetSkipForUndo(true);
#else
	HeeksObj::operator=(b);
	*A = *b.A;
	*B = *b.B;
#endif
	return *this;
}

HeeksObj* EndedObject::MakeACopyWithID()
{
#ifdef MULTIPLE_OWNERS
	EndedObject* pnew = (EndedObject*)ObjList::MakeACopyWithID();
	pnew->A = (HPoint*)pnew->GetFirstChild();
	pnew->B = (HPoint*)pnew->GetNextChild();
#else
	EndedObject* pnew = (EndedObject*)HeeksObj::MakeACopyWithID();
#endif
	return pnew;
}

bool EndedObject::IsDifferent(HeeksObj *other)
{
	EndedObject* eobj = (EndedObject*)other;
	if(eobj->A->m_p.Distance(A->m_p) > wxGetApp().m_geom_tol)
		return true;

	if(eobj->B->m_p.Distance(B->m_p) > wxGetApp().m_geom_tol)
		return true;

	return HeeksObj::IsDifferent(other);
}

#ifdef MULTIPLE_OWNERS
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
#endif

void EndedObject::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A->m_p.Transform(mat);
	B->m_p.Transform(mat);
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
#ifdef MULTIPLE_OWNERS
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(object->OnVisibleLayer() && object->m_visible)
		{
			bool object_marked = wxGetApp().m_marked_list->ObjectMarked(object);
			if(object->GetType() == PointType && !select && !object_marked)continue; // don't show points unless the point object is selected.
			if(select)glPushName(object->GetIndex());
#ifdef HEEKSCAD
			(*It)->glCommands(select, marked || wxGetApp().m_marked_list->ObjectMarked(object), no_color);
#else
			(*It)->glCommands(select, marked || heeksCAD->ObjectMarked(object), no_color);
#endif
			if(select)glPopName();
		}
	}
#endif
}

/*void EndedObject::WriteBaseXML(TiXmlElement *element)
{
	HeeksObj::WriteBaseXML(element);
}*/

