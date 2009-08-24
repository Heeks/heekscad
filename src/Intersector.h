// Intersector.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

class FastCurve
{
public:
	double addedAt;

	FastCurve(){}
	~FastCurve(){}
	virtual void Reverse()=0;
	virtual double GetY()=0;
	virtual double GetY(double ax)=0;
	virtual	double GetU(double x, double y)=0;
	virtual double GetYatU(double u)=0;
	virtual double GetXatU(double u)=0;
};

class FastLine: public FastCurve
{
public:
	gp_Pnt A;
	gp_Pnt B;
	FastLine(gp_Pnt A,gp_Pnt B){this->A = A; this->B = B;}
	FastLine(){}
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

	double GetXatU(double u)
	{
		double dx = B.X() - A.X();
		return u * dx + A.X();
	}

	double GetYatU(double u)
	{
		double dy = B.Y() - A.Y();
		return u * dy + A.Y();
	}
};

class Intersection
{
public:
	FastCurve* curve;
	double X,Y;
	Intersection(FastCurve* curve, double X, double Y){this->curve = curve; this->X = X; this->Y = Y;}
};


class Intersector{
public:
	Intersector(){}
	~Intersector(){}
	virtual std::map<FastCurve*, std::vector<Intersection> > Intersect(std::vector<FastCurve*> &lines)=0;
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
