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
}

EndedObject::EndedObject(const EndedObject& e)
{
	operator=(e);
}

EndedObject::~EndedObject(){
}

const EndedObject& EndedObject::operator=(const EndedObject &b){
	HeeksObj::operator=(b);
	A = b.A;
	B = b.B;
	return *this;
}

HeeksObj* EndedObject::MakeACopyWithID()
{
	EndedObject* pnew = (EndedObject*)HeeksObj::MakeACopyWithID();
	return pnew;
}

bool EndedObject::IsDifferent(HeeksObj *other)
{
	EndedObject* eobj = (EndedObject*)other;
	if(eobj->A.Distance(A) > wxGetApp().m_geom_tol)
		return true;

	if(eobj->B.Distance(B) > wxGetApp().m_geom_tol)
		return true;

	return HeeksObj::IsDifferent(other);
}

void EndedObject::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A.Transform(mat);
	B.Transform(mat);
}

bool EndedObject::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(data == &A){
		A = vp.XYZ() + vshift.XYZ();
	}
	else if(data == &B){
		B = vp.XYZ() + vshift.XYZ();
	}
	return false;
}

void EndedObject::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	list->push_back(GripData(GripperTypeStretch,A.X(),A.Y(),A.Z(),&A));
	list->push_back(GripData(GripperTypeStretch,B.X(),B.Y(),B.Z(),&B));
}

bool EndedObject::GetStartPoint(double* pos)
{
	extract(A, pos);
	return true;
}

bool EndedObject::GetEndPoint(double* pos)
{
	extract(B, pos);
	return true;
}
