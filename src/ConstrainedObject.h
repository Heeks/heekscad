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

	ConstrainedObject();
	~ConstrainedObject(void);

	void SetAbsoluteAngleConstraint(EnumAbsoluteAngle angle);
	void SetPerpendicularConstraint(ConstrainedObject* obj);
	void SetParallelConstraint(ConstrainedObject* obj);
	void glCommands(HeeksColor color, gp_Ax1 mid_point);
	void RemoveExisting(ConstrainedObject* obj);
	bool HasConstraints();
};
