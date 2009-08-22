// MultiPoly.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

#include "BentleyOttmann.h"

void MultiPoly(std::list<CSketch*> sketches);

class BoundedCurve
{
public:
	MyLine* line;
	double startu,endu;
	BoundedCurve(MyLine *line, double startu, double endu){this->line = line; this->startu = startu; this->endu = endu;}

	double GetAX()
	{
		double dx = line->B.X() - line->A.X();
		return startu * dx + line->A.X();
	}

	double GetAY()
	{
		double dy = line->B.Y() - line->A.Y();
		return startu * dy + line->A.Y();
	}
	
	double GetBX()
	{
		double dx = line->B.X() - line->A.X();
		return endu * dx + line->A.X();
	}

	double GetBY()
	{
		double dy = line->B.Y() - line->A.Y();
		return endu * dy + line->A.Y();
	}

	gp_Pnt Begin()
	{
		return gp_Pnt(GetAX(),GetAY(),0);
	}

	gp_Pnt End()
	{
		return gp_Pnt(GetBX(),GetBY(),0);
	}

};

enum WhichEnd
{
	FirstEnd,
	LastEnd,
	NoEnd
};

enum WhichPoint
{
	PointA,
	PointB
};

class CompoundSegment
{
public:
	BoundedCurve *firstline;
	WhichPoint firstpoint;
	BoundedCurve *lastline;
	WhichPoint lastpoint;
	std::list<BoundedCurve*> lines;
	std::vector<WhichPoint> points;
	double m_tol;
	CompoundSegment(){}
	CompoundSegment(MyLine *line, double tol, double startu, double endu)
	{
		firstline = new BoundedCurve(line,startu,endu);
		lastline=firstline;
		lines.push_back(firstline);
		firstpoint = PointA;
		lastpoint = PointB;
		m_tol = tol;
	}
	~CompoundSegment(){}
	void Reverse() 
	{
		BoundedCurve* tline=firstline; 
		firstline=lastline;
		lastline=tline;
		WhichPoint tpoint = firstpoint;
		firstpoint = lastpoint;
		lastpoint = tpoint;
	}

	WhichEnd GetWhichEnd(double atx, double aty)
	{
		if(firstpoint == PointA)
		{
			if(MyIsEqual(firstline->GetAX(),atx) && MyIsEqual(firstline->GetAY(),aty))
				return FirstEnd;
		}
		else
		{
			if(MyIsEqual(firstline->GetBX(),atx) && MyIsEqual(firstline->GetBY(),aty))
				return FirstEnd;  
		}

		if(lastpoint == PointA)
		{
		
			if(MyIsEqual(lastline->GetAX(),atx) && MyIsEqual(lastline->GetAY(),aty))
				return LastEnd;
		}
		else
		{
			if(MyIsEqual(lastline->GetBX(),atx) && MyIsEqual(lastline->GetBY(),aty))
				return LastEnd;
		}
		return FirstEnd; //added to remove warning. I assume this should never happen should we be returning an error value
	}

	gp_Pnt Begin()
	{
		if(firstpoint == PointA)
			return firstline->Begin();
		else
			return firstline->End();
	}

	gp_Pnt End()
	{
		if(lastpoint == PointA)
			return lastline->Begin();
		else
			return lastline->End();
	}

	double GetArea(BoundedCurve* c1, WhichPoint dir1, BoundedCurve* c2, WhichPoint dir2)
	{
		gp_Pnt gPnt1 = c1->Begin();
		if(dir1 == PointB)
			gPnt1 = c1->End();

		gp_Pnt gPnt2 = c2->Begin();
		if(dir2 == PointB)
			gPnt2 = c2->End();

		return gPnt1.X()*gPnt2.Y()-gPnt2.X()*gPnt1.Y();
	}

	double GetArea()
	{
		double total = 0;
		std::list<BoundedCurve*>::iterator it=lines.begin();
		std::list<BoundedCurve*>::iterator it2=lines.begin();
		int idx = 0;
		for(it2++; it2 != lines.end(); ++it2)
		{
			total += GetArea(*it++,points[idx],*it2,points[idx+1]);
			idx++;
		}
		total+=GetArea(*it,points[idx],*lines.begin(),points[0]);
		return total*.5;
	}

	double GetWindingNumber(gp_Pnt pnt)
	{
		//TODO: we probably need an ordering of all lines in the segment to do this
		return 0;
	}

	void Order()
	{
		//TODO: is it possible that the segments are not in a logical order?
		gp_Pnt lastpoint = Begin();
		std::list<BoundedCurve*>::iterator it;
		for(it = lines.begin(); it!= lines.end(); ++it)
		{
			if((*it)->Begin().Distance(lastpoint) <= m_tol)
			{
				points.push_back(PointA);
				lastpoint = (*it)->End();
			}
			else
			{
				points.push_back(PointB);
				lastpoint = (*it)->Begin();
			}
		}
	}

	void Add(CompoundSegment* seg, double atx, double aty)
	{
		std::list<BoundedCurve*>::iterator it;

		WhichEnd myend = GetWhichEnd(atx,aty);
		WhichEnd oend = seg->GetWhichEnd(atx,aty);

		if(myend == FirstEnd)
		{
			if(oend == FirstEnd)
			{
				firstline = seg->lastline;
				firstpoint = seg->lastpoint;
				seg->lines.reverse();
			}
			else
			{
				firstline = seg->firstline;
				firstpoint = seg->firstpoint;
			}

			for(it = seg->lines.begin(); it!= seg->lines.end(); ++it)
			{
				lines.push_front(*it);
			}

		}
		else
		{
			if(oend == FirstEnd)
			{
				lastline = seg->lastline;
				lastpoint = seg->lastpoint;
			}
			else
			{
				lastline = seg->firstline;
				lastpoint = seg->firstpoint;

				seg->lines.reverse();
			}
			for(it = seg->lines.begin(); it!= seg->lines.end(); ++it)
			{
				lines.push_back(*it);
			}
		}
	}
};


