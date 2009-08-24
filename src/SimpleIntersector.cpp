// SimpleIntersector.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Intersector.h"
#include "SimpleIntersector.h"

extern double tol; 

struct IntersectionSort
{
     bool operator()(const Intersection &pStart, const Intersection& pEnd)
     {
		 return pStart.curve->GetU(pStart.X,pStart.Y) < pEnd.curve->GetU(pEnd.X,pEnd.Y);
     }
};

std::map<FastCurve*, std::vector<Intersection> > SimpleIntersector::Intersect(std::vector<FastCurve*> &lines)
{
	tol = wxGetApp().m_geom_tol;
	std::map<FastCurve*, std::vector<Intersection> > intersections;

	for(unsigned i=0; i < lines.size(); i++)
	{
		for(unsigned j=i+1; j < lines.size(); j++)
		{
			//TODO: switch intersector based on curve type
			IntResult ir = Intersects((FastLine*)lines[i],(FastLine*)lines[j]);
			if(ir.exists)
			{
				intersections[lines[i]].push_back(Intersection(lines[j],ir.atX,ir.atY));
				intersections[lines[j]].push_back(Intersection(lines[i],ir.atX,ir.atY));
			}
		}
	}

	//Sort the intersections by U
	std::map<FastCurve*, std::vector<Intersection> >::iterator it;
	for(it = intersections.begin(); it != intersections.end(); ++it)
	{
		std::sort((*it).second.begin(),(*it).second.end(),IntersectionSort());
	}

	return intersections;
}

IntResult SimpleIntersector::Intersects(FastLine* line1, FastLine* line2)
{
	//Checks if these lines intersect somewhere besides the start point. End point is reported.
	//if(line1->A.IsEqual(line2->A,tol) || line1->B.IsEqual(line2->B,tol))
	//	return IntResult(false,0);

	double ua = (line2->B.X()-line2->A.X())*(line1->A.Y()-line2->A.Y()) - (line2->B.Y()-line2->A.Y())*(line1->A.X()-line2->A.X());
	ua /= (line2->B.Y()-line2->A.Y())*(line1->B.X()-line1->A.X())-(line2->B.X()-line2->A.X())*(line1->B.Y()-line1->A.Y());
	double ub = (line1->B.X()-line1->A.X())*(line1->A.Y()-line2->A.Y()) - (line1->B.Y()-line1->A.Y())*(line1->A.X()-line2->A.X());
	ub /= (line2->B.Y()-line2->A.Y())*(line1->B.X()-line1->A.X())-(line2->B.X()-line2->A.X())*(line1->B.Y()-line1->A.Y());

	if(ua > -tol && ua < 1+tol && ub > -tol && ub < 1+tol)
	{
		double atX = line1->A.X() + ua * (line1->B.X() - line1->A.X());
		double atY = line1->A.Y() + ua * (line1->B.Y() - line1->A.Y());
		return IntResult(true,ua,ub,atX,atY);
	}
	return IntResult(false,0,0,0,0);
}
