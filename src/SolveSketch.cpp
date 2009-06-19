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

std::vector<point> params;


void SolveSketch(CSketch* sketch)
{
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

						line l = GetLineFromEndedObject(eobj);

						if(cobj->absoluteangleconstraint)
						{
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
							//TODO: check if constraint is already processed
							//need check hashcode

							Constraint *con = *it;
							if(cons.find(con)!=cons.end())
								continue;

							switch(con->m_type)
							{
								case CoincidantPointConstraint:
									break;
								case ParallelLineConstraint:
									{
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

	if(!solve((double*)&params[0],params.size(),&constraints[0],constraints.size(),1))
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
	}
	
}

line GetLineFromEndedObject(EndedObject* eobj)
{
		point p;
		p.x = &eobj->ax; p.y = &eobj->ay;
		params.push_back(p);
		point p2;
		p2.x = &eobj->bx; p2.y = &eobj->by;
		params.push_back(p2);

		line l;
		l.p1 = p;
		l.p2 = p2;

		return l;
}