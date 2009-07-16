// BentleyOttmann.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "BentleyOttmann.h"

//This algorithm takes an array of polygon segments and computes the intersection points
//using a modified Bentley-Ottmann algorithm. There are several problems with the text book
//approach. Such as, not supporting vertical lines. Coincident points, or points with the same
//X coordinate. The usual way to alleviate these problems is to skew the data in someway which
//prevents the algorithm from breaking. We take a different approach and make the algorithm work
//will the ill-conditioned data. There is a cost to this. I'm not totally sure what it is. 
//It is something like instead of being (n+k)log n. we are (n+k+m)log n+m. where m is the number
//of ill conditioned points. This means there is a worst case complexity of this algorithm which is higher
//than n^2. 

//TODO: I wanted to have these "fuzzy trees" which would insert items into new lists depending on there being within
//tolerance of the other elements. This seemed to break the stl::map for some unknown reason. Instead, the index
//values are rounded to some multiple of tol. This guarantees that 2 items within the list are within tol. But items
//from an immediately adjacent list, could have members within tol of some items. This is not checked for, and should be

//TODO: The minimal x-step is dependant on the slope of the lines in question. Otherwise it is possible to take a step
//small enough to leave lines still intersecting. There may be another way to handle this.


double currentX=0;

//fast storage for the global tolerance paramater
double tol=0;

//Storage for the event table
std::map<double,std::vector<std::list<MyLine*> > > eventtable;

std::map<MyLine*, std::vector<Intersection> > Intersections(std::vector<MyLine> &lines)
{
	tol = wxGetApp().m_geom_tol;
	eventtable.clear();

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
	std::map<double,std::set<MyLine*> > sweepline;

	//Storage for the located intersections
	std::map<MyLine*, std::vector<Intersection> >intersections;

	//Storage for the already intersected code
	std::map<MyLine*,std::set<MyLine*> > intersected;

	//Go through the event points in order
	std::map<double,std::vector<std::list<MyLine*> > >::iterator it;
	for(it = eventtable.begin(); it != eventtable.end(); ++it)
	{
		std::list<MyLine*>::iterator it2;
		MyLine* tline=0;
		currentX = (*it).first;

		for(it2 = (*it).second[IntersectionType].begin(); it2 != (*it).second[IntersectionType].end(); ++it2)
		{
			//Remove these items from the list and re-add them
			tline = *it2;
			double xSave = currentX;
			currentX = tline->addedAt;
			double currentY = tline->GetY();
			double loc = MyRound(currentY);
			sweepline[loc].erase(sweepline[loc].find(tline));
			if(sweepline[loc].size() == 0)
				sweepline.erase(sweepline.find(loc));
			currentX = xSave;
			tline->addedAt = currentX;
			currentY = tline->GetY();
			loc = MyRound(currentY);
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
			double loc = MyRound(currentY);
			sweepline[loc].insert(tline);
		}

		//try to break apart coincident point sets. 
		//TODO: should erase the list in sweepline if it is empty
		std::map<double,std::set<MyLine*> >::iterator it3;
		for(it3 = sweepline.begin(); it3 != sweepline.end(); it3++)
		{
			std::set<MyLine*> lines = (*it3).second;
			if(lines.size() <= 1)
				continue;
			double baseloc = (*it3).first;
			std::set<MyLine*>::iterator it4;
			for(it4 = lines.begin(); it4 != lines.end();)
			{
				MyLine* tline = *it4;
				double currentY = tline->GetY(currentX);
				double newloc = MyRound(currentY);
				if(newloc!=baseloc)
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
				intersections[tline].push_back(Intersection(tline,currentX,tline->GetY(currentX)));
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
			double loc = MyRound(currentY);
			sweepline[loc].erase(sweepline[loc].find(tline));
			if(sweepline[loc].size() == 0)
				sweepline.erase(sweepline.find(loc));
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
	return intersections;
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

//Put a new event into the table
void InsertEvent(EventType type, double x, MyLine* line)
{
	//Find an existing list, or a place to insert it
	double  pos = MyRound(x);
	if(eventtable[pos].size() == 0)
		eventtable[pos].resize(4);

	eventtable[pos][type].push_back(line);
}

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

//Determine if two doubles are the same within tolerance
inline bool MyIsEqual(double a, double b)
{
	if(a > b - tol && a < b + tol)
		return true;
	return false;
}

//Round double to the nearest multiple of tol
//TODO: is this right? maybe use fmod(d,tol)?
double MyRound(double d)
{
	return floor(d/tol) * tol;
}
