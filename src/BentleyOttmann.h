// BentleyOttmann.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

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

	double GetU(double x, double y)
	{
		double dx = A.X() - B.X();
		double dy = A.Y() - B.Y();

		double t=-(A.X()*dx-x*dx+A.Y()*dy-y*dy)/(dx*dx+dy*dy);
		return t;
	}
};

class Intersection
{
public:
	MyLine* line;
	double X,Y;
	Intersection(MyLine* line, double X, double Y){this->line = line; this->X = X; this->Y = Y;}
};

enum EventType
{
	AddType,
	RemoveType,
	AddRemoveType,
	IntersectionType
};

class IntResult
{
public:
	bool exists;
	double atX;
	IntResult(bool exists, double atX){this->exists = exists; this->atX = atX;}
};

inline bool MyIsEqual(double a, double b);
bool MyIsEqual2(MyLine* line1, MyLine* line2, double at);
double MyRound(double d);
IntResult Intersects(MyLine* line1, MyLine* line2);
void InsertEvent(EventType type, double x, MyLine* line);
std::map<MyLine*, std::vector<Intersection> > Intersections(std::vector<MyLine> &lines);
