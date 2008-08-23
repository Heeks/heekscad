// DigitizedPoint.cpp

#include "stdafx.h"
#include "DigitizedPoint.h"
#include "HCircle.h"
#include "HLine.h"
#include "HArc.h"
#include "HILine.h"

DigitizedPoint::DigitizedPoint()
{
	m_type = DigitizeNoItemType;
	m_point = gp_Pnt(0, 0, 0);
	m_object1 = NULL;
	m_object2 = NULL;
}

DigitizedPoint::DigitizedPoint(gp_Pnt point, DigitizeType t, HeeksObj* object1, HeeksObj* object2)
{
	m_point = point;
	m_type = t;
	m_object1 = object1;
	m_object2 = object2;
}

int DigitizedPoint::importance(){
	switch(m_type){
	case DigitizeEndofType:
		return 10;

	case DigitizeIntersType:
		return 5;

	case DigitizeMidpointType:
		return 7;

	case DigitizeCentreType:
		return 7;

	case DigitizeNearestType:
		return 4;

	case DigitizeTangentType:
		return 3;

	default:
		return 0;
	}
}

static void GetLineOrCircleType(const DigitizedPoint& d, int &type, gp_Lin &l, gp_Circ &c)
{
	type = UnknownType;

	if(d.m_object1)
	{
		switch(d.m_object1->GetType())
		{
		case LineType:
			type = LineType;
			l = ((HLine*)(d.m_object1))->GetLine();
			break;
		case ILineType:
			type = LineType;
			l = ((HILine*)(d.m_object1))->GetLine();
			break;
		case ArcType:
			type = CircleType;
			c = ((HArc*)(d.m_object1))->m_circle;
			break;
		case CircleType:
			type = CircleType;
			c = ((HCircle*)(d.m_object1))->m_circle;
			break;
		}
	}
}

// static member function
bool DigitizedPoint::GetLinePoints(const DigitizedPoint& d1, const DigitizedPoint& d2, gp_Pnt &p1, gp_Pnt &p2)
{
	// calculate tangent points
	p1 = d1.m_point;
	p2 = d2.m_point;

	gp_Circ c1, c2;
	gp_Lin l1, l2;
	int type1, type2; // UnknownType, LineType or CircleType

	GetLineOrCircleType(d1, type1, l1, c1);
	GetLineOrCircleType(d2, type2, l2, c2);

	if(d1.m_type == DigitizeTangentType && d2.m_type == DigitizeTangentType)
	{
		if(type1 == CircleType && type2 == CircleType)
		{
			return HCircle::GetLineTangentPoints(c1, c2, d1.m_point, d2.m_point, p1, p2);
		}
	}
	else if(d1.m_type == DigitizeTangentType)
	{
		if(type1 == CircleType)
		{
			return HCircle::GetLineTangentPoint(c1, d1.m_point, d2.m_point, p1);
		}
	}
	else if(d2.m_type == DigitizeTangentType)
	{
		if(type2 == CircleType)
		{
			return HCircle::GetLineTangentPoint(c2, d2.m_point, d1.m_point, p2);
		}
	}

	return false;
}

// static member function
bool DigitizedPoint::GetArcPoints(const DigitizedPoint& d1, const gp_Vec *initial_direction, const DigitizedPoint& d2, gp_Pnt &p1, gp_Pnt &p2, gp_Pnt &centre, gp_Dir &axis)
{
	// calculate tangent points
	p1 = d1.m_point;
	p2 = d2.m_point;

	gp_Circ c1, c2;
	gp_Lin l1, l2;
	int type1, type2; // UnknownType, LineType or CircleType

	GetLineOrCircleType(d1, type1, l1, c1);
	GetLineOrCircleType(d2, type2, l2, c2);

	if(d1.m_type == DigitizeTangentType && d2.m_type == DigitizeTangentType)
	{
		if(type1 == CircleType && type2 == CircleType)
		{
			bool success = HCircle::GetArcTangentPoints(c1, c2, d1.m_point, d2.m_point, wxGetApp().digitizing_radius, p1, p2, centre, axis);
			if(success && initial_direction){
				// get the axis the right way round
			}
			return success;
		}
		if(type1 == LineType && type2 == CircleType)
		{
			return HCircle::GetArcTangentPoints(c2, l1, d2.m_point, wxGetApp().digitizing_radius, p1, p2, centre, axis);
		}
		if(type1 == CircleType && type2 == LineType)
		{
			return HCircle::GetArcTangentPoints(c1, l2, d1.m_point, wxGetApp().digitizing_radius, p2, p1, centre, axis);
		}
		if(type1 == LineType && type2 == LineType)
		{
			return HCircle::GetArcTangentPoints(l1, l2, d1.m_point, d2.m_point, wxGetApp().digitizing_radius, p1, p2, centre, axis);
		}
		return false;
	}
	else if(d1.m_type == DigitizeTangentType)
	{
		if(type1 == CircleType)
		{
			return HCircle::GetArcTangentPoint(c1, d1.m_point, d2.m_point, initial_direction, NULL, p1, centre, axis);
		}
		else if(type1 == LineType)
		{
			return HCircle::GetArcTangentPoint(l1, d1.m_point, d2.m_point, initial_direction, NULL, p1, centre, axis);
		}
		return false;
	}
	else if(d2.m_type == DigitizeTangentType)
	{
		gp_Vec minus_dir;
		if(initial_direction)minus_dir = -(*initial_direction);
		if(type2 == CircleType)
		{
			return HCircle::GetArcTangentPoint(c2, d2.m_point, d1.m_point, (initial_direction != NULL) ? (&minus_dir):NULL, NULL, p2, centre, axis);
		}
		else if(type2 == LineType)
		{
			return HCircle::GetArcTangentPoint(l2, d2.m_point, d1.m_point, (initial_direction != NULL) ? (&minus_dir):NULL, NULL, p2, centre, axis);
		}
		return false;
	}

	p1 = d1.m_point;
	p2 = d2.m_point;

	return true;
}
