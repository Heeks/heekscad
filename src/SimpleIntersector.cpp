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
			IntResult ir = Intersects(lines[i],lines[j]);

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

IntResult SimpleIntersector::Intersects(FastCurve* curve1, FastCurve* curve2)
{
	FastLine* line1 = dynamic_cast<FastLine*>(curve1);
	if(line1)
	{
		FastLine* line2 = dynamic_cast<FastLine*>(curve2);
		if(line2)
			return LinesIntersect((FastLine*)curve1,(FastLine*)curve2);
		else
		{
			FastArc* arc2 = dynamic_cast<FastArc*>(curve2);
			if(arc2)
			{
				return LineArcIntersect(line1,arc2);
			}
		}
	}
	else
	{
		FastArc* arc1 = dynamic_cast<FastArc*>(curve1);
		if(arc1)
		{
			FastLine* line2 = dynamic_cast<FastLine*>(curve2);
			if(line2)
			{
				return LineArcIntersect(line2,arc1);
			}
			else
			{
				FastArc* arc2 = dynamic_cast<FastArc*>(curve2);
			}
		}
	}

	return IntResult(false,0,0,0,0);
}

IntResult SimpleIntersector::LinesIntersect(FastLine* line1, FastLine* line2)
{
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

IntResult SimpleIntersector::LineArcIntersect(FastLine* line, FastArc* arc)
{

	double dx = line->B.X() - line->A.X();
	double dy = line->B.Y() - line->A.Y();
	double a = dx*dx + dy*dy;
	double b = 2 * ((line->B.X() - line->A.X()) * (line->A.X() - arc->C.X()) + (line->B.Y() - line->A.Y()) * (line->A.Y() - arc->C.Y())); 
	double c = arc->C.X() * arc->C.X() + arc->C.Y() * arc->C.Y() + line->A.X()*line->A.X() + line->A.Y()*line->A.Y() - 2*(arc->C.X() * line->A.X() + arc->C.Y() * line->A.Y()) - arc->rad * arc->rad;
/*	
	double ua = (line2->B.X()-line2->A.X())*(line1->A.Y()-line2->A.Y()) - (line2->B.Y()-line2->A.Y())*(line1->A.X()-line2->A.X());
	ua /= (line2->B.Y()-line2->A.Y())*(line1->B.X()-line1->A.X())-(line2->B.X()-line2->A.X())*(line1->B.Y()-line1->A.Y());
	double ub = (line1->B.X()-line1->A.X())*(line1->A.Y()-line2->A.Y()) - (line1->B.Y()-line1->A.Y())*(line1->A.X()-line2->A.X());
	ub /= (line2->B.Y()-line2->A.Y())*(line1->B.X()-line1->A.X())-(line2->B.X()-line2->A.X())*(line1->B.Y()-line1->A.Y());

	if(ua > -tol && ua < 1+tol && ub > -tol && ub < 1+tol)
	{
		double atX = line1->A.X() + ua * (line1->B.X() - line1->A.X());
		double atY = line1->A.Y() + ua * (line1->B.Y() - line1->A.Y());
		return IntResult(true,ua,ub,atX,atY);
	}*/
	return IntResult(false,0,0,0,0);
}
