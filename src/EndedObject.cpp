// EndedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "EndedObject.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyVertex.h"
#include "../tinyxml/tinyxml.h"

EndedObject::EndedObject(){
}

EndedObject::~EndedObject(){
}

const EndedObject& EndedObject::operator=(const EndedObject &b){
	HeeksObj::operator=(b);
	A = b.A;
	B = b.B;
	return *this;
}

void EndedObject::LoadToDoubles()
{
	ax = A.X(); ay = A.Y();
	bx = B.X(); by = B.Y();
}

void EndedObject::LoadFromDoubles()
{
	A = gp_Pnt(ax,ay,0);
	B = gp_Pnt(bx,by,0);
}

bool EndedObject::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A.Transform(mat);
	B.Transform(mat);
	return false;
}

bool EndedObject::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(!data){
		A = vp.XYZ() + vshift.XYZ();
	}
	else if(data == (void*)1){
		B = vp.XYZ() + vshift.XYZ();
	}
	return false;
}

void EndedObject::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	list->push_back(GripData(GripperTypeStretch,A.X(),A.Y(),A.Z(),0));
	list->push_back(GripData(GripperTypeStretch,B.X(),B.Y(),B.Z(),(void*)1));
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


