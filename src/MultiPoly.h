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
	CompoundSegment(){}
	CompoundSegment(MyLine *line, double startu, double endu)
	{
		firstline = new BoundedCurve(line,startu,endu);
		lastline=firstline;
		lines.push_back(firstline);
		firstpoint = PointA;
		lastpoint = PointB;
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

	void Add(CompoundSegment* seg, double atx, double aty)
	{
		std::list<BoundedCurve*>::iterator it;
		for(it = seg->lines.begin(); it!= seg->lines.end(); ++it)
		{
			lines.push_back(*it);
		}

		WhichEnd myend = GetWhichEnd(atx,aty);
		WhichEnd oend = seg->GetWhichEnd(atx,aty);

		if(myend == FirstEnd)
		{
			if(oend == FirstEnd)
			{
				firstline = seg->lastline;
				firstpoint = seg->lastpoint;
			}
			else
			{
				firstline = seg->firstline;
				firstpoint = seg->firstpoint;
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
				lastpoint = seg->lastpoint;
			}
		}
	}
};



