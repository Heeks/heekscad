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
	AbsoluteAngleConstraint
};

enum EnumAbsoluteAngle{
	AbsoluteAngleHorizontal,
	AbsoluteAngleVertical
};


class Constraint{
public:
	HeeksObj* m_obj1;
	HeeksObj* m_obj2;

	EnumConstraintType m_type;
	EnumAbsoluteAngle m_angle;

	Constraint();
	Constraint(EnumConstraintType,EnumAbsoluteAngle,HeeksObj* obj);
	~Constraint(void);

	bool operator==(const Constraint &other) const {
		return m_type == other.m_type && m_angle==other.m_angle && m_obj1 == other.m_obj1 && m_obj2 == other.m_obj2;
	}

	void glCommands(HeeksColor color, gp_Ax1 mid_point);
	void render_text(const wxChar* str);
};
