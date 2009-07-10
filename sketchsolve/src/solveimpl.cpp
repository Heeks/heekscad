/*
 * solve.cpp
 *
 *  Created on: May 4, 2009
 *      Author: Jonathan George
 *      This
 *      Copyright (c) 2009, Jonathan George
 *      This program is released under the BSD license. See the file COPYING for details.
 *
 */

#include "solve.h"
#include <cmath>
#include <stdlib.h>
#include <sstream>

using namespace std;

void SolveImpl::LoadDouble(std::list<std::pair<varLocation,void*> > &mylist, double *d)
{
	if(parms[d])
	{
		//many vars may already be in the vector, must check and remap
		if(mapset.find(d) != mapset.end())
		{
			mylist.push_back(mapparms[d]);
			return;
		}
        std::pair<varLocation,void*> newloc(Vector,(void*)next_vector++);
		mylist.push_back(newloc);
		mapparms[d] = newloc;
		mapset.insert(d);
		return;
	}

	mylist.push_back(std::pair<varLocation,void*>(Static,d));
}

void SolveImpl::LoadPoint(std::list<std::pair<varLocation,void*> > &mylist, point p)
{
	LoadDouble(mylist,p.x);
	LoadDouble(mylist,p.y);
}

void SolveImpl::LoadLine(std::list<std::pair<varLocation,void*> > &mylist,line l)
{
	LoadPoint(mylist,l.p1);
	LoadPoint(mylist,l.p2);
}

void SolveImpl::LoadArc(std::list<std::pair<varLocation,void*> > &mylist,arc a)
{
	LoadPoint(mylist,a.center);
	LoadDouble(mylist,a.startAngle);
	LoadDouble(mylist,a.endAngle);
	LoadDouble(mylist,a.rad);
}

void SolveImpl::LoadCircle(std::list<std::pair<varLocation,void*> > &mylist,circle c)
{
	LoadPoint(mylist,c.center);
	LoadDouble(mylist,c.rad);
}

SolveImpl::SolveImpl()
{
	next_vector=0;
	registerdependency(parallel,line1);
	registerdependency(parallel,line2);
	registerconstraint(parallel,ParallelError);

	registerdependency(horizontal,line1);
	registerconstraint(horizontal,HorizontalError);

	registerdependency(vertical,line1);
	registerconstraint(vertical,VerticalError);

	registerdependency(pointOnPoint,point1);
	registerdependency(pointOnPoint,point2);
	registerconstraint(pointOnPoint,PointOnPointError);

	registerdependency(P2PDistance,point1);
	registerdependency(P2PDistance,point2);
	registerdependency(P2PDistance,parameter);
	registerconstraint(P2PDistance,P2PDistanceError);

}

SolveImpl::~SolveImpl()
{
}

double SolveImpl::GetError()
{
	std::vector<double> myvec;
	double error = 0;

	std::list<constraintType>::iterator it;
	std::list<std::list<std::pair<varLocation,void*> > >::iterator it2 = constraintvars.begin();
	for(it = constrainttypes.begin(); it != constrainttypes.end(); ++it)
	{
		myvec.clear();
		std::list<std::pair<varLocation,void*> > tlist = *it2;
		std::list<std::pair<varLocation,void*> >::iterator it3;
		for(it3 = tlist.begin(); it3 != tlist.end(); ++it3)
		{
			std::pair<varLocation,void*> tvar = *it3;
			if(tvar.first == Vector)
				myvec.push_back(GetElement((int)tvar.second));
			else
				myvec.push_back(*((double*)tvar.second));
		}
		error += errors[*it](myvec);
		++it2;
	}
	return error;
}

void SolveImpl::Load(constraint &c)
{
	std::list<std::pair<varLocation,void*> > mylist;
	std::list<dependencyType>::iterator it;
	for(it = dependencies[c.type].begin(); it != dependencies[c.type].end(); ++it)
	{
		switch(*it)
		{
			case line1: LoadLine(mylist,c.line1); break;
			case line1_p1: LoadPoint(mylist,c.line1.p1); break;
			case line1_p1_x: LoadDouble(mylist,c.line1.p1.x); break;
			case line1_p1_y: LoadDouble(mylist,c.line1.p1.y); break;
			case line1_p2: LoadPoint(mylist,c.line2.p2); break;
			case line1_p2_x: LoadDouble(mylist,c.line1.p2.x); break;
			case line1_p2_y: LoadDouble(mylist,c.line1.p2.y); break;
			case line2: LoadLine(mylist,c.line2); break;
			case line2_p1: LoadPoint(mylist,c.line2.p1); break;
			case line2_p1_x: LoadDouble(mylist,c.line2.p1.x); break;
			case line2_p1_y: LoadDouble(mylist,c.line2.p1.y); break;
			case line2_p2: LoadPoint(mylist,c.line2.p2); break;
			case line2_p2_x: LoadDouble(mylist,c.line2.p2.x); break;
			case line2_p2_y: LoadDouble(mylist,c.line2.p2.y); break;
			case point1: LoadPoint(mylist,c.point1); break;
			case point2: LoadPoint(mylist,c.point2); break;
			case parameter: LoadDouble(mylist,c.parameter); break;
			case arc1: LoadArc(mylist,c.arc1); break;
			case arc1_rad: LoadDouble(mylist,c.arc1.rad); break;
			case arc1_startAngle: LoadDouble(mylist,c.arc1.startAngle); break;
			case arc1_endAngle: LoadDouble(mylist,c.arc1.endAngle); break;
			case arc1_center: LoadPoint(mylist,c.arc1.center); break;
			case arc1_center_x: LoadDouble(mylist,c.arc1.center.x); break;
			case arc1_center_y: LoadDouble(mylist,c.arc1.center.y); break;
			case arc2: LoadArc(mylist,c.arc2); break;
			case arc2_rad: LoadDouble(mylist,c.arc2.rad); break;
			case arc2_startAngle: LoadDouble(mylist,c.arc2.startAngle); break;
			case arc2_endAngle: LoadDouble(mylist,c.arc2.endAngle); break;
			case arc2_center: LoadPoint(mylist,c.arc2.center); break;
			case arc2_center_x: LoadDouble(mylist,c.arc2.center.x); break;
			case arc2_center_y: LoadDouble(mylist,c.arc2.center.y); break;
			case circle1: LoadCircle(mylist,c.circle1); break;
			case circle1_rad: LoadDouble(mylist,c.circle1.rad); break;
			case circle1_center: LoadPoint(mylist,c.circle1.center); break;
			case circle1_center_x: LoadDouble(mylist,c.circle1.center.x); break;
			case circle1_center_y: LoadDouble(mylist,c.circle1.center.y); break;
			case circle2: LoadCircle(mylist,c.circle2); break;
			case circle2_rad: LoadDouble(mylist,c.circle2.rad); break;
			case circle2_center: LoadPoint(mylist,c.circle2.center); break;
			case circle2_center_x: LoadDouble(mylist,c.circle2.center.x); break;
			case circle2_center_y: LoadDouble(mylist,c.circle2.center.y); break;
		}
	}
	constraintvars.push_back(mylist);
	constrainttypes.push_back(c.type);
}

void SolveImpl::Unload()
{
	//For every item in mapparms, copy variable from vector into pointer
	std::map<double*,std::pair<varLocation,void*> >::iterator it;
	for(it = mapparms.begin(); it != mapparms.end(); ++it)
	{
		std::pair<varLocation, void*> parm = (*it).second;
		double* location = (*it).first;

		if(parm.first != Vector)
			continue;

		*location = GetElement((int)parm.second);
	}
}

void SolveImpl::registerconstraint(constraintType type,double(*error)(std::vector<double>))
{
	if(errors.size() < (int)type + 1)
		errors.resize((int)type+1);
	errors[type] = error;
}

void SolveImpl::registerdependency(constraintType type, dependencyType d)
{
	if(depset.find(type) == depset.end())
	{
		if(dependencies.size() < (int)type + 1)
			dependencies.resize((int)type+1);
		depset.insert(type);
	}
	dependencies[type].push_back(d);
}
