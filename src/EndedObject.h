// EndedObject.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include "HPoint.h"
#ifdef MULTIPLE_OWNERS
#include "../interface/ObjList.h"

class EndedObject: public ObjList{
#else
class EndedObject: public HeeksObj{
#endif
public:
	HPoint* A, *B;

	~EndedObject(void);
	EndedObject(const HeeksColor* color);
#ifndef MULTIPLE_OWNERS
	EndedObject(const EndedObject& e);
#endif

	const EndedObject& operator=(const EndedObject &b);

#ifdef MULTIPLE_OWNERS
	virtual void LoadToDoubles();
	virtual void LoadFromDoubles();
#endif

	// HeeksObj's virtual functions
	bool Stretch(const double *p, const double* shift, void* data);
	void ModifyByMatrix(const double* m);
	bool GetStartPoint(double* pos);
	bool GetEndPoint(double* pos);
	void CopyFrom(const HeeksObj* object){operator=(*((EndedObject*)object));}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void glCommands(bool select, bool marked, bool no_color);
	HeeksObj* MakeACopyWithID();
	bool IsDifferent(HeeksObj* other);
//	void WriteBaseXML(TiXmlElement *element);
	
};
