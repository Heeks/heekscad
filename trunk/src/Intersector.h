// Intersector.h
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

		double t=(A.X()*dx-x*dx+A.Y()*dy-y*dy)/(dx*dx+dy*dy);
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


class Intersector{
public:
	Intersector(){}
	~Intersector(){}
	virtual std::map<MyLine*, std::vector<Intersection> > Intersect(std::vector<MyLine> &lines)=0;
};

class IntResult
{
public:
	bool exists;
	double atX;
	double atY;
	double uA;
	double uB;
	IntResult(bool exists, double uA, double uB, double atX, double atY){this->exists = exists; this->atX = atX;this->atY = atY; this->uA = uA; this->uB = uB;}
};
