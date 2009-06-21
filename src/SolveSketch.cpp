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


line GetLineFromEndedObject(EndedObject* eobj);
point GetPoint(EndedObject* eobj, EnumPoint point);

std::vector<double*> params;


void SolveSketch(CSketch* sketch)
{
	params.clear();
	std::list<line> lines;
	std::vector<constraint> constraints;
	std::set<Constraint*> cons;

	HeeksObj* obj = sketch->GetFirstChild();
	while(obj)
	{
		ConstrainedObject* cobj = (ConstrainedObject*)obj;
		EndedObject *eobj = (EndedObject*)obj;
		if(cobj && cobj->HasConstraints())
		{
			switch(obj->GetType())
			{
				case LineType:
					{
						eobj->LoadToDoubles();

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
								case CoincidantPointConstraint:
									break;
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
											EndedObject* eobj2 = (EndedObject*)con->m_obj2;
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
					break;
				default:
					break;
			}
		}
		obj = sketch->GetNextChild();
	}

	if(solve(&params[0],params.size(),&constraints[0],constraints.size(),rough))
		//No result
		return;

	obj = sketch->GetFirstChild();
	while(obj)
	{
		ConstrainedObject* cobj = (ConstrainedObject*)obj;
		EndedObject *eobj = (EndedObject*)obj;
		if(cobj && cobj->HasConstraints())
		{
			eobj->LoadFromDoubles();
		}
		obj = sketch->GetNextChild();
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

		params.push_back(p.x);
		params.push_back(p.y);

		return p;
}

line GetLineFromEndedObject(EndedObject* eobj)
{
		eobj->LoadToDoubles();
		point p;
		p.x = &eobj->ax; p.y = &eobj->ay;
		params.push_back(p.x);
		params.push_back(p.y);
		point p2;
		p2.x = &eobj->bx; p2.y = &eobj->by;
		params.push_back(p2.x);
		params.push_back(p2.y);

		line l;
		l.p1 = p;
		l.p2 = p2;

		return l;
}