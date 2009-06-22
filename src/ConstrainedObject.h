// ConstrainedObject.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class Constraint;

#include "../interface/HeeksObj.h"
#include "Constraint.h"

class ConstrainedObject: public HeeksObj{
public:
	std::list<Constraint*> constraints;
	Constraint* absoluteangleconstraint;
	Constraint*  linelengthconstraint;

	ConstrainedObject();
	~ConstrainedObject(void);

	void SetAbsoluteAngleConstraint(EnumAbsoluteAngle angle);
	void SetPerpendicularConstraint(ConstrainedObject* obj);
	void SetParallelConstraint(ConstrainedObject* obj);
	void glCommands(HeeksColor color, gp_Ax1 mid_point);
	void RemoveExisting(ConstrainedObject* obj);
	bool HasConstraints();
	void SetCoincidentPoint(ConstrainedObject* obj,EnumPoint obj1_point,EnumPoint obj2_point);
	bool HasPointConstraint(ConstrainedObject* obj,EnumPoint obj1_point,EnumPoint obj2_point);
	void SetLineLengthConstraint(double length);
	void SetLineLength(double length);
};
