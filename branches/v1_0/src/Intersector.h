// Intersector.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

//TODO: get this value from somewhere else
#define TOLERANCE .0001

#include "FastCurves.h"

class RayIntersection
{
public:
	double u;
	gp_Pnt pnt;
	bool bounded;
	bool lower;
	RayIntersection(double u, gp_Pnt pnt, bool bounded, bool lower){this->u = u; this->pnt = pnt; this->bounded=bounded; this->lower=lower;};
	~RayIntersection(){};
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
	void Swap()
	{
		double temp = uA;
		uA = uB;
		uB = temp;
	}	
};
