// FastCurves.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class RayIntersection;

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
	virtual std::vector<RayIntersection> RayIntersects(gp_Pnt p)=0;
};


class FastLine: public FastCurve
{
public:
	gp_Pnt A;
	gp_Pnt B;
	FastLine(gp_Pnt A,gp_Pnt B);
	FastLine();
	void Reverse();
	double GetY();
	double GetY(double ax);
	double GetU(double x, double y);
	double GetXatU(double u);
	double GetYatU(double u);
	std::vector<RayIntersection> RayIntersects(gp_Pnt pnt);
};

class FastArc: public FastCurve
{
public:
	gp_Pnt A;
	gp_Pnt B;
	gp_Pnt C;
	double a1;
	double a2;
	double da;
	double rad;
	bool cw;
	bool m_flipped;
	gp_Circ m_circ; 

	FastArc(gp_Pnt A,gp_Pnt B, gp_Pnt C, bool cw, gp_Circ circ);
	FastArc();
	gp_Circ GetCircle();
	void Reverse();
	double GetY();
	double GetY(double ax);
	double GetU(double x, double y);
	double GetXatU(double u);
	double GetYatU(double u);
	double TransformY(double y);
	double TransformX(double x);
	std::vector<RayIntersection> RayIntersects(gp_Pnt p);
};
