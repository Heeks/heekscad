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
			std::vector<IntResult> results = Intersects(lines[i],lines[j]);
			
			for(unsigned int k=0; k < results.size(); k++)
			{
				IntResult ir = results[k];
				if(ir.exists)
				{
					intersections[lines[i]].push_back(Intersection(lines[j],ir.atX,ir.atY));
					intersections[lines[j]].push_back(Intersection(lines[i],ir.atX,ir.atY));
				}
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

std::vector<IntResult> SimpleIntersector::Intersects(FastCurve* curve1, FastCurve* curve2)
{
	std::vector<IntResult> ret;
	bool swap=false;
	FastLine* line1 = dynamic_cast<FastLine*>(curve1);
	if(line1)
	{
		FastLine* line2 = dynamic_cast<FastLine*>(curve2);
		if(line2)
			ret = LinesIntersect((FastLine*)curve1,(FastLine*)curve2);
		else
		{
			FastArc* arc2 = dynamic_cast<FastArc*>(curve2);
			if(arc2)
			{
				ret = LineArcIntersect(line1,arc2);
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
				ret = LineArcIntersect(line2,arc1);
				swap = true;
			}
			else
			{
				FastArc* arc2 = dynamic_cast<FastArc*>(curve2);
				ret = ArcsIntersect(arc1,arc2);
			}
		}
	}

	if(swap)
	{
		for(unsigned int i=0; i < ret.size(); i++)
		{
			ret[i].Swap();
		}
	}

	return ret;
}

std::vector<IntResult> SimpleIntersector::LinesIntersect(FastLine* line1, FastLine* line2)
{
	std::vector<IntResult> ret;
	double ua = (line2->B.X()-line2->A.X())*(line1->A.Y()-line2->A.Y()) - (line2->B.Y()-line2->A.Y())*(line1->A.X()-line2->A.X());
	ua /= (line2->B.Y()-line2->A.Y())*(line1->B.X()-line1->A.X())-(line2->B.X()-line2->A.X())*(line1->B.Y()-line1->A.Y());
	double ub = (line1->B.X()-line1->A.X())*(line1->A.Y()-line2->A.Y()) - (line1->B.Y()-line1->A.Y())*(line1->A.X()-line2->A.X());
	ub /= (line2->B.Y()-line2->A.Y())*(line1->B.X()-line1->A.X())-(line2->B.X()-line2->A.X())*(line1->B.Y()-line1->A.Y());

	if(ua > -tol && ua < 1+tol && ub > -tol && ub < 1+tol)
	{
		double atX = line1->A.X() + ua * (line1->B.X() - line1->A.X());
		double atY = line1->A.Y() + ua * (line1->B.Y() - line1->A.Y());
		ret.push_back(IntResult(true,ua,ub,atX,atY));
	}
	return ret;
}

std::vector<IntResult> SimpleIntersector::LineArcIntersect(FastLine* line, FastArc* arc)
{
	std::vector<IntResult> ret;

	double dx = line->B.X() - line->A.X();
	double dy = line->B.Y() - line->A.Y();
	double a = dx*dx + dy*dy;
	double b = 2 * ((line->B.X() - line->A.X()) * (line->A.X() - arc->C.X()) + (line->B.Y() - line->A.Y()) * (line->A.Y() - arc->C.Y())); 
	double c = arc->C.X() * arc->C.X() + arc->C.Y() * arc->C.Y() + line->A.X()*line->A.X() + line->A.Y()*line->A.Y() - 2*(arc->C.X() * line->A.X() + arc->C.Y() * line->A.Y()) - arc->rad * arc->rad;

	double det = b * b - 4 * a * c;
	if(det<-tol)
		return ret;

	if(det<0)
		det = 0;

	if(det > -tol && det < tol)
	{
		//single intersection
		double u = (-b + sqrt(det)) / (2 * a);
		ret.push_back(LineArcIntersect(line,arc,u));
		return ret;
	}

	//These u's are for the line
	double u1 = (-b + sqrt(det)) / (2 * a);
	double u2 = (-b - sqrt(det)) / (2 * a);

	ret.push_back(LineArcIntersect(line,arc,u1));
	ret.push_back(LineArcIntersect(line,arc,u2));

	return ret;
}

IntResult SimpleIntersector::LineArcIntersect(FastLine* line, FastArc* arc, double u)
{
	if(u < -tol || u > 1 + tol)
		return IntResult(false,0,0,0,0);

	double x = line->GetXatU(u);
	double y = line->GetYatU(u);

	double arcu = arc->GetU(x,y);

	if(arcu < -tol || arcu > 1 + tol)
		return IntResult(false,0,0,0,0);

	return IntResult(true,u,arcu,x,y);
}

std::vector<IntResult> SimpleIntersector::ArcsIntersect(FastArc* arc1, FastArc* arc2)
{
	std::vector<IntResult> ret;

	double d = arc1->C.Distance(arc2->C);

	if(d - tol > arc1->rad + arc2->rad)
		return ret;
	if(d + tol < fabs(arc1->rad - arc2->rad))
		return ret;

	if(d < tol && d > -tol && fabs(arc1->rad -arc2->rad) < tol)
		return ret; //coincident

	double a = (arc1->rad * arc1->rad - arc2->rad * arc2->rad + d * d) / (2 * d);
	double h = sqrt(arc1->rad * arc1->rad - a * a);
	if(h!=h)
		h=0;

	double xl = arc1->C.X() + a * ( arc2->C.X() - arc1->C.X()) / d;
	double yl = arc1->C.Y() + a * ( arc2->C.Y() - arc1->C.Y()) / d;

	double x1 = xl + h * ( arc2->C.Y() - arc1->C.Y() ) / d;
	double y1 = yl - h * ( arc2->C.X() - arc1->C.X() ) / d;

	double x2 = xl - h * ( arc2->C.Y() - arc1->C.Y() ) / d;
	double y2 = yl + h * ( arc2->C.X() - arc1->C.X() ) / d;

	if(d > arc1->rad + arc2->rad - tol && d < arc1->rad + arc2->rad + tol)
	{
		//single solution
		ret.push_back(ArcsIntersect(arc1,arc2,x1,y1));
		return ret;
	}

	ret.push_back(ArcsIntersect(arc1,arc2,x1,y1));
	ret.push_back(ArcsIntersect(arc1,arc2,x2,y2));

	return ret;

}

IntResult SimpleIntersector::ArcsIntersect(FastArc* arc1, FastArc* arc2, double x, double y)
{
	double arc1u = arc1->GetU(x,y);

	if(arc1u < -tol || arc1u > 1 + tol)
		return IntResult(false,0,0,0,0);

	double arc2u = arc1->GetU(x,y);

	if(arc2u < -tol || arc2u > 1 + tol)
		return IntResult(false,0,0,0,0);

	return IntResult(true,arc1u,arc2u,x,y);
}

