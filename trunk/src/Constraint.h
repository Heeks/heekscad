// ConstrainedObject.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"


class ConstrainedObject;

#define ANGLE_OFFSET_FROM_LINE 2.5

enum EnumConstraintType{
	CoincidantPointConstraint,
	ParallelLineConstraint,
	PerpendicularLineConstraint,
	AbsoluteAngleConstraint,
	LineLengthConstraint,
	LineTangentConstraint,
	RadiusConstraint,
	EqualLengthConstraint,
	ColinearConstraint,
	EqualRadiusConstraint,
	ConcentricConstraint,
	PointOnLineConstraint,
	PointOnLineMidpointConstraint,
	PointOnArcMidpointConstraint,
	PointOnArcConstraint
};

enum EnumAbsoluteAngle{
	AbsoluteAngleHorizontal,
	AbsoluteAngleVertical
};

class Constraint : public HeeksObj{
public:
	ConstrainedObject* m_obj1;
	ConstrainedObject* m_obj2;

	EnumConstraintType m_type;
	EnumAbsoluteAngle m_angle;

	double m_length;

	Constraint();
	Constraint(const Constraint* obj);
	Constraint(EnumConstraintType,EnumAbsoluteAngle,ConstrainedObject* obj);
	Constraint(EnumConstraintType,double length,ConstrainedObject* obj);
	Constraint(EnumConstraintType,ConstrainedObject* obj1, ConstrainedObject* obj2);
	Constraint(EnumConstraintType,EnumAbsoluteAngle,double length,ConstrainedObject* obj1, ConstrainedObject* obj2);

	~Constraint(void);

	HeeksObj *MakeACopy(void)const;
	int GetType()const{return ConstraintType;}
	const wxChar* GetTypeString(void)const{return _("Constraint");}
	wxString GetIcon(){return wxGetApp().GetResFolder() + _T("/icons/line");}
	void WriteXML(TiXmlNode *root);
	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	static void BeginSave();
	static void EndSave(TiXmlNode *root);

	bool operator==(const Constraint &other) const {
		return m_type == other.m_type && m_angle==other.m_angle && m_obj1 == other.m_obj1 && m_obj2 == other.m_obj2 && m_length == other.m_length;
	}

	const Constraint& operator=(const Constraint &b);

	void glCommands(HeeksColor color, gp_Ax1 mid_point);
	void render_text(const wxChar* str);
};
