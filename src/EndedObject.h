// EndedObject.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"

class EndedObject: public HeeksObj{
protected:
	HeeksColor color;

public:
	gp_Pnt A, B;

	~EndedObject(void);
	EndedObject(void);
	EndedObject(const EndedObject& e);

	const EndedObject& operator=(const EndedObject &b);

	// HeeksObj's virtual functions
	bool Stretch(const double *p, const double* shift, void* data);
	void ModifyByMatrix(const double* m);
	bool GetStartPoint(double* pos);
	bool GetEndPoint(double* pos);
	void CopyFrom(const HeeksObj* object){operator=(*((EndedObject*)object));}
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	HeeksObj* MakeACopyWithID();
	bool IsDifferent(HeeksObj* other);
	void WriteBaseXML(TiXmlElement *element);
	void ReadBaseXML(TiXmlElement* element);
};
