// ConstrainedObject.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"

#define ANGLE_OFFSET_FROM_LINE 2.5

enum EnumConstraintType{
	CoincidantPointConstraint,
	ParallelLineConstraint,
	PerpendicularLineConstraint,
	AbsoluteAngleConstraint,
	LineLengthConstraint
};

enum EnumAbsoluteAngle{
	AbsoluteAngleHorizontal,
	AbsoluteAngleVertical
};

enum EnumPoint{
	PointA,
	PointB
};

class Constraint{
public:
	HeeksObj* m_obj1;
	HeeksObj* m_obj2;

	EnumPoint m_obj1_point;
	EnumPoint m_obj2_point;

	EnumConstraintType m_type;
	EnumAbsoluteAngle m_angle;

	double m_length;

	Constraint();
	Constraint(EnumConstraintType,EnumAbsoluteAngle,HeeksObj* obj);
	Constraint(EnumConstraintType,double length,HeeksObj* obj);
	Constraint(EnumConstraintType,EnumPoint,EnumPoint,HeeksObj* obj1, HeeksObj* obj2);

	~Constraint(void);

	bool operator==(const Constraint &other) const {
		return m_type == other.m_type && m_angle==other.m_angle && m_obj1 == other.m_obj1 && m_obj2 == other.m_obj2 && m_obj1_point == other.m_obj1_point && m_obj2_point == other.m_obj2_point && m_length == other.m_length;
	}

	void glCommands(HeeksColor color, gp_Ax1 mid_point);
	void render_text(const wxChar* str);
};
