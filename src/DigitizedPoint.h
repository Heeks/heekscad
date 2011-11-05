// DigitizedPoint.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once


enum DigitizeType{
	DigitizeNoItemType,
	DigitizeEndofType,
	DigitizeIntersType,
	DigitizeMidpointType,
	DigitizeCentreType,
	DigitizeScreenType,
	DigitizeCoordsType,
	DigitizeNearestType,
	DigitizeTangentType,
	DigitizeInputType // typed into properties, for example
};

class DigitizedPoint{
public:
	gp_Pnt m_point;
	DigitizeType m_type;
	HeeksObj* m_object1;
	HeeksObj* m_object2;

	DigitizedPoint();
	DigitizedPoint(gp_Pnt point, DigitizeType t, HeeksObj* object1 = NULL, HeeksObj* object2 = NULL);

	int importance();

	// calculate tangent points
	static bool GetLinePoints(const DigitizedPoint& d1, const DigitizedPoint& d2, gp_Pnt &p1, gp_Pnt &p2);
	static bool GetArcPoints(const DigitizedPoint& d1, const gp_Vec *initial_direction, const DigitizedPoint& d2, gp_Pnt &p1, gp_Pnt &p2, gp_Pnt &centre, gp_Dir &axis);
	static bool GetTangentCircle(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, gp_Circ& c);
	static bool GetEllipse(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, gp_Elips& e);
	static bool GetCircleBetween(const DigitizedPoint& d1, const DigitizedPoint& d2, gp_Circ& c);
	static bool GetQuarticSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, Handle_Geom_BSplineCurve &spline);
	static bool GetCubicSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline);
	static bool GetRationalSpline(std::list<DigitizedPoint> &spline_points, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline);
};

