// BentleyOttmann.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

//This algorithm takes an array of polygon segments and computes the intersection points

class MyLine
{
public:
	gp_Pnt A;
	gp_Pnt B;
	double addedAt;
	MyLine(gp_Pnt A,gp_Pnt B){this->A = A; this->B = B;}
	MyLine(){}
	void Reverse() {gp_Pnt tmp = A; A = B; B = tmp;}
	double GetY()
	{
		double dy = B.Y() - A.Y();
		double dx = B.X() - A.X();

		double t = (addedAt - A.X()) / dx;
		return A.Y() + t * dy;
	}

	double GetY(double ax)
	{
		double dy = B.Y() - A.Y();
		double dx = B.X() - A.X();

		double t = (ax - A.X()) / dx;
		return A.Y() + t * dy;
	}
};

class Intersection
{
public:
	MyLine* line;
	double X,Y;
	Intersection(MyLine* line, double X, double Y){this->line = line; this->X = X; this->Y = Y;}
};

double currentX=0;

class LineCmp
{
public:
	MyLine line;
	double ofst;

	LineCmp(MyLine* line, double ofst){this->line = *line; this->ofst = ofst;}

	double GetY()
	{
		return line.GetY() + ofst;
	}

	double GetY(double ax)
	{
		return line.GetY(ax) + ofst;
	}

};

//fast storage for the global tolerance paramater
double tol=0;

//Determine if two doubles are the same within tolerance
inline bool MyIsEqual(double a, double b)
{
	if(a > b - tol && a < b + tol)
		return true;
	return false;
}

enum EventType
{
	AddType,
	RemoveType,
	AddRemoveType,
	IntersectionType
};

//By using an std::pair and this comparison function. We can get std::map to group operations in the red black tree 
//while taking into account geometric tolerance. the first and second parameters have already been perturbed by the
//tolerance value.
class IsLessThan
{
public:
	bool operator()(std::pair<double,double> a, std::pair<double,double> b)
	{
		return a.second < b.first;
	}

	bool operator()(std::pair<LineCmp,LineCmp> a, std::pair<LineCmp,LineCmp> b)
	{
		return a.second.GetY() < b.first.GetY();
	}

	bool operator()(std::pair<LineCmp,LineCmp> a, std::pair<LineCmp,LineCmp> b, double ax)
	{
		return a.second.GetY(ax) < b.first.GetY(ax);
	}

};

class IntResult
{
public:
	bool exists;
	double atX;
	IntResult(bool exists, double atX){this->exists = exists; this->atX = atX;}
};

IntResult Intersects(MyLine* line1, MyLine* line2)
{
	//Checks if these lines intersect somewhere besides the start and end points
	if(line1->A.IsEqual(line2->A,tol) || line1->B.IsEqual(line2->B,tol))
		return IntResult(false,0);

	double ua = (line2->B.X()-line2->A.X())*(line1->A.Y()-line2->A.Y()) - (line2->B.Y()-line2->A.Y())*(line1->A.X()-line2->A.X());
	ua /= (line2->B.Y()-line2->A.Y())*(line1->B.X()-line1->A.X())-(line2->B.X()-line2->A.X())*(line1->B.Y()-line1->A.Y());
	double ub = (line1->B.X()-line1->A.X())*(line1->A.Y()-line2->A.Y()) - (line1->B.Y()-line1->A.Y())*(line1->A.X()-line2->A.X());
	ub /= (line2->B.Y()-line2->A.Y())*(line1->B.X()-line1->A.X())-(line2->B.X()-line2->A.X())*(line1->B.Y()-line1->A.Y());

	if(ua > 0 && ua < 1 && ub > 0 && ub < 1)
	{
		double atX = line1->A.X() + ua * (line1->B.X() - line1->A.X());
		return IntResult(true,atX);
	}
	return IntResult(false,0);
}

//Storage for the event table
std::map<std::pair<double,double>,std::vector<std::list<MyLine*> >,IsLessThan> eventtable;

//Put a new event into the table
void InsertEvent(EventType type, double x, MyLine* line)
{
	//Find an existing list, or a place to insert it
	std::pair<double,double> pos(x-tol,x+tol);
	if(eventtable[pos].size() == 0)
		eventtable[pos].resize(4);

	eventtable[pos][type].push_back(line);
}

void Intersections(std::vector<MyLine> &lines)
{
	tol = wxGetApp().m_geom_tol;

	//first pass: build lists ordered by x coordinate
	for(size_t i=0; i < lines.size(); i++)
	{
		//We must do a binary search on the list, as we want to enfore a geom_tol
		if(MyIsEqual(lines[i].A.X(),lines[i].B.X()))
		{
			InsertEvent(AddRemoveType,lines[i].A.X(),&lines[i]); //For vertical lines
		}
		else
		{
			if(lines[i].A.X() > lines[i].B.X())
				lines[i].Reverse();
			InsertEvent(AddType,lines[i].A.X(),&lines[i]);
			InsertEvent(RemoveType,lines[i].B.X(),&lines[i]);
		}
	}

	//Storage for the sweepline
	std::map<std::pair<double,double>,std::set<MyLine*>,IsLessThan> sweepline;
	IsLessThan ILT;

	//Storage for the located intersections
	std::vector<Intersection> intersections;

	//Storage for the already intersected code
	std::map<MyLine*,std::set<MyLine*> > intersected;

	//Go through the event points in order
	std::map<std::pair<double,double>,std::vector<std::list<MyLine*> >,IsLessThan >::iterator it;
	for(it = eventtable.begin(); it != eventtable.end(); ++it)
	{
		std::list<MyLine*>::iterator it2;
		MyLine* tline=0;
		currentX = (*it).first.first + tol;

		for(it2 = (*it).second[IntersectionType].begin(); it2 != (*it).second[IntersectionType].end(); ++it2)
		{
			//Remove these items from the list and re-add them
			tline = *it2;
			double xSave = currentX;
			currentX = tline->addedAt;
			double currentY = tline->GetY();
			std::pair<double,double> loc(currentY-tol,currentY+tol);
			sweepline[loc].erase(sweepline[loc].find(tline));
			currentX = xSave;
			tline->addedAt = currentX;
			currentY = tline->GetY();
			loc = std::pair<double,double>(currentY-tol,currentY+tol);
			sweepline[loc].insert(tline);
		}

		for(it2 = (*it).second[AddType].begin(); it2 != (*it).second[AddType].end(); ++it2)
		{
			//Add these new items to the sweepline
			tline = *it2;
			tline->addedAt = currentX;
			//No numerical problems here, because if A.X and B.X were within tol. Line would be
			//in a different list
			double currentY = tline->GetY();
			std::pair<double,double> loc(currentY-tol,currentY+tol);
			sweepline[loc].insert(tline);
		}

		//try to break apart coincident point sets. 
		std::map<std::pair<double,double>,std::set<MyLine*>,IsLessThan>::iterator it3;
		for(it3 = sweepline.begin(); it3 != sweepline.end(); it3++)
		{
			std::set<MyLine*> lines = (*it3).second;
			if(lines.size() <= 1)
				continue;
			std::pair<double,double> baseloc = (*it3).first;
			std::set<MyLine*>::iterator it4;
			for(it4 = lines.begin(); it4 != lines.end();)
			{
				MyLine* tline = *it4;
				double currentY = tline->GetY(currentX);
				std::pair<double,double> newloc(currentY-tol,currentY+tol);
				if(ILT(newloc,baseloc) || ILT(baseloc,newloc))
				{
					std::set<MyLine*>::iterator it5 = it4;
					++it4;
					double oldx = currentX;
					currentX = tline->addedAt;
					(*it3).second.erase(tline);
					currentX = oldx;
					tline->addedAt = currentX;
					sweepline[newloc].insert(tline);
					continue;
				}
				++it4;
			}
		}

		//TODO: all sets in sweepline represent intersections
		for(it3 = sweepline.begin(); it3 != sweepline.end(); it3++)
		{
			std::set<MyLine*> lines = (*it3).second;
			if(lines.size() <= 1)
				continue;
			//All the elements of lines are intersecting at this X
			std::set<MyLine*>::iterator it4;
			for(it4 = lines.begin(); it4 != lines.end(); ++it4)
			{
				tline = *it4;
				intersections.push_back(Intersection(tline,currentX,tline->GetY(currentX)));
			}
		}

		for(it2 = (*it).second[AddRemoveType].begin(); it2 != (*it).second[AddRemoveType].end(); ++it2)
		{
			//TODO: These intersect with all things between y1 and y2
		}

		for(it2 = (*it).second[RemoveType].begin(); it2 != (*it).second[RemoveType].end(); ++it2)
		{
			//Remove these from the sweepline
			tline = *it2;
			double xSave = currentX;
			currentX = tline->addedAt;
			double currentY = tline->GetY();
			std::pair<double,double> loc(currentY-tol,currentY+tol);
			sweepline[loc].erase(sweepline[loc].find(tline));
			currentX = xSave;
		}

		//TODO::Run intersection tests for between all adjacent sets. Then add these to the event tree
		for(it3 = sweepline.begin(); it3 != sweepline.end();)
		{
			std::set<MyLine*> lines = (*it3++).second;
			if(it3 == sweepline.end())
				break;
			std::set<MyLine*> nlines = (*it3).second;

			//Compare all lines in lines to each other and to all lines in nlines
			std::set<MyLine*>::iterator it4;
			for(it4 = lines.begin(); it4 != lines.end(); ++it4)
			{
				std::set<MyLine*>::iterator it5;
				for(it5 = nlines.begin(); it5 != nlines.end(); ++it5)
				{
					//Do the comparisons
					if(intersected[*it4].find(*it5) != intersected[*it4].end())
						continue;
					IntResult ir = Intersects(*it4,*it5);
					if(ir.exists && ir.atX > currentX + tol)
					{
						InsertEvent(IntersectionType,ir.atX,*it4);
						InsertEvent(IntersectionType,ir.atX,*it5);
						intersected[*it4].insert(*it5);
						intersected[*it5].insert(*it4);
					}
				}
				//TODO: I don't think it is necessary to compute the intersection of things within a group
				//as they are alread scheduled for regrouping
				/*
				it5 = it4;
				for(it5++; it5 != lines.end(); ++it5)
				{
					//Do the comparisons
					IntResult ir = Intersects(*it4,*it5);
					if(ir.exists)
					{
						InsertEvent(IntersectionType,ir.atX,*it4);
						InsertEvent(IntersectionType,ir.atX,*it5);
					}
				} */

			}
		}
	}
}

void Test()
{
	std::vector<MyLine> lines;
	lines.push_back(MyLine(gp_Pnt(0,0,0),gp_Pnt(5,5,0)));
	lines.push_back(MyLine(gp_Pnt(0,0,0),gp_Pnt(2,5,0)));
	lines.push_back(MyLine(gp_Pnt(0,1,0),gp_Pnt(4,1,0)));
//	lines.push_back(MyLine(gp_Pnt(1,1,0),gp_Pnt(1,2,0)));
	Intersections(lines);
}