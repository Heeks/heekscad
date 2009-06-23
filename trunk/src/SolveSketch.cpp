// SolveSketch.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sketch.h"
#include "../interface/ObjList.h"
#include "SolveSketch.h"
#include "ConstrainedObject.h"
#include "EndedObject.h"
#include "../sketchsolve/src/solve.h"
#include "HArc.h"

arc GetArc(HArc* a);
line GetLineFromEndedObject(EndedObject* eobj);
point GetPoint(EndedObject* eobj, EnumPoint point);

std::vector<double*> params;
std::set<double*> paramset;


void SolveSketch(CSketch* sketch)
{
	params.clear();
	paramset.clear();
	std::list<line> lines;
	std::vector<constraint> constraints;
	std::set<Constraint*> cons;

	HeeksObj* obj = sketch->GetFirstChild();
	while(obj)
	{
		ConstrainedObject* cobj = (ConstrainedObject*)obj;
		EndedObject *eobj = (EndedObject*)obj;
		if(cobj)
		{
			eobj->LoadToDoubles();
			if(obj->GetType() == ArcType)
			{
				arc a = GetArc((HArc*)obj);
				constraint c;
				c.type = arcRules;
				c.arc1 = a;
				constraints.push_back(c);
			}

			if(cobj->absoluteangleconstraint)
			{
				line l = GetLineFromEndedObject(eobj);
				constraint c;
				c.line1 = l;
				c.type = vertical;
				if(cobj->absoluteangleconstraint->m_angle == AbsoluteAngleHorizontal)
					c.type = horizontal;
				constraints.push_back(c);
			}

			if(cobj->linelengthconstraint)
			{
				line l = GetLineFromEndedObject(eobj);
				constraint c;
				c.line1 = l;
				c.type = lineLength;
				c.parameter = &cobj->linelengthconstraint->m_length;
				constraints.push_back(c);
			}

			if(cobj->radiusconstraint)
			{
				arc a = GetArc((HArc*)obj);
				constraint c;
				c.arc1 = a;
				c.type = radiusValue;
				c.parameter = &cobj->radiusconstraint->m_length;
				constraints.push_back(c);
			}
	
			std::list<Constraint*>::iterator it;
			for(it = cobj->constraints.begin(); it!=cobj->constraints.end(); ++it)
			{
				//check if constraint is already processed
				//uses set of pointers for easy comparison
				Constraint *con = *it;
				if(cons.find(con)!=cons.end())
					continue;

				switch(con->m_type)
				{
					case LineTangentToArcConstraint:
					{
						arc a = GetArc((HArc*)obj);
						constraint c;
						c.type = tangentToArc;
						c.arc1 = a;
						c.line1 = GetLineFromEndedObject((EndedObject*)con->m_obj2);
						constraints.push_back(c);
					}
					break;
					case CoincidantPointConstraint:
					{
						constraint c;
						c.type = pointOnPoint;
								
						if((ConstrainedObject*)con->m_obj1 == cobj)
						{
							EndedObject* eobj2 = (EndedObject*)con->m_obj2;
							c.point1 = GetPoint(eobj,con->m_obj1_point);
								
							c.point2 = GetPoint(eobj2,con->m_obj2_point);
						}
						else
						{
							EndedObject* eobj2 = (EndedObject*)con->m_obj1;
							c.point2 = GetPoint(eobj2, con->m_obj1_point);
											
							c.point1 = GetPoint(eobj,con->m_obj2_point);
						}
						constraints.push_back(c);
						cons.insert(con);
					}
					break;
					case ParallelLineConstraint:
					{
						line l = GetLineFromEndedObject(eobj);
						constraint c;
						c.type = parallel;
						c.line1 = l;
											
						if((ConstrainedObject*)con->m_obj1 == cobj)
							c.line2 = GetLineFromEndedObject((EndedObject*)con->m_obj2);
						else
							c.line2 = GetLineFromEndedObject((EndedObject*)con->m_obj1);
						constraints.push_back(c);
						cons.insert(con);
					}
					break;
					case PerpendicularLineConstraint:
					{
						line l = GetLineFromEndedObject(eobj);
						constraint c;
						c.type = perpendicular;
						c.line1 = l;
	
						if((ConstrainedObject*)con->m_obj1 == cobj)
							c.line2 = GetLineFromEndedObject((EndedObject*)con->m_obj2);
						else
							c.line2 = GetLineFromEndedObject((EndedObject*)con->m_obj1);
						constraints.push_back(c);
						cons.insert(con);
					}
					break;
				}
			}
		}
		obj = sketch->GetNextChild();
	}

	if(solve(&params[0],params.size(),&constraints[0],constraints.size(),fine))
		//No result
		return;

	obj = sketch->GetFirstChild();
	while(obj)
	{
		ConstrainedObject* cobj = (ConstrainedObject*)obj;
		EndedObject *eobj = (EndedObject*)obj;
		if(cobj)
		{
			eobj->LoadFromDoubles();
		}
		obj = sketch->GetNextChild();
	}
	
}

void PushBack(double *v)
{
	if(paramset.find(v) == paramset.end())
	{
		params.push_back(v);
		paramset.insert(v);
	}
}

point GetPoint(EndedObject* obj, EnumPoint whichpoint)
{
		obj->LoadToDoubles();
		point p;
		if(whichpoint == PointA)
		{
			p.x = &obj->ax; p.y = &obj->ay;
		}
		else
		{
			p.x = &obj->bx; p.y = &obj->by;
		}

		PushBack(p.x);
		PushBack(p.y);

		return p;
}

line GetLineFromEndedObject(EndedObject* eobj)
{
		eobj->LoadToDoubles();
		point p;
		p.x = &eobj->ax; p.y = &eobj->ay;
		PushBack(p.x);
		PushBack(p.y);
		point p2;
		p2.x = &eobj->bx; p2.y = &eobj->by;
		PushBack(p2.x);
		PushBack(p2.y);

		line l;
		l.p1 = p;
		l.p2 = p2;

		return l;
}

arc GetArc(HArc* a)
{
	arc ret;
	ret.start = GetPoint(a,PointA);
	ret.end = GetPoint(a,PointB);

	point p;
	p.x = &a->cx;
	p.y = &a->cy;

	PushBack(p.x);
	PushBack(p.y);

	ret.center = p;

	return ret;
}