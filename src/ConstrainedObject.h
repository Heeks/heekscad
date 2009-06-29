// ConstrainedObject.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class Constraint;

#include "../interface/HeeksObj.h"
#include "Constraint.h"
class HPoint;

class ConstrainedObject: public ObjList{
public:
	std::list<Constraint*> constraints;
	Constraint* absoluteangleconstraint;
	Constraint*  linelengthconstraint;
	Constraint* radiusconstraint;

	ConstrainedObject();
	~ConstrainedObject(void);

	const ConstrainedObject& operator=(const ConstrainedObject &b);

	virtual void LoadToDoubles();
	virtual void LoadFromDoubles();


	void SetAbsoluteAngleConstraint(EnumAbsoluteAngle angle);
	bool SetPerpendicularConstraint(ConstrainedObject* obj);
	bool SetParallelConstraint(ConstrainedObject* obj);
	bool SetEqualLengthConstraint(ConstrainedObject* obj);
	bool SetColinearConstraint(ConstrainedObject* obj);
	bool SetConcentricConstraint(ConstrainedObject* obj);
	bool SetEqualRadiusConstraint(ConstrainedObject* obj);
	void glCommands(HeeksColor color, gp_Ax1 mid_point);
	bool RemoveExisting(HeeksObj* obj, EnumConstraintType type);
	bool HasConstraints();
	void SetCoincidentPoint(ConstrainedObject* obj, bool remove);
	bool HasPointConstraint(ConstrainedObject* obj);
	void SetLineLengthConstraint(double length);
	void SetRadiusConstraint(double radius);
	void SetLineLength(double length);
	void SetRadius(double radius);
	void SetTangentConstraint(ConstrainedObject* obj);
	void SetPointOnLineConstraint(HPoint* obj);
	void SetPointOnLineMidpointConstraint(HPoint* obj);
	void SetPointOnArcConstraint(HPoint* obj);
	void SetPointOnArcMidpointConstraint(HPoint* obj);

};
