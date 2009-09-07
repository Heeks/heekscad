// Intersector.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

//TODO: get this value from somewhere else
#define TOLERANCE .0001

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
	FastLine(gp_Pnt A,gp_Pnt B){this->A = A; this->B = B;}
	FastLine(){}
	void Reverse() {};//gp_Pnt tmp = A; A = B; B = tmp;}
	double GetY()
	{
		return GetY(addedAt);
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
#ifdef ROUNDU
		if(t > -TOLERANCE && t < TOLERANCE)
			t = 0;
		if(t > 1 - TOLERANCE && t < 1 + TOLERANCE)
			t = 1;
#endif
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

	std::vector<RayIntersection> RayIntersects(gp_Pnt pnt)
	{
		std::vector<RayIntersection> ret;
		if((pnt.Y() < B.Y() + TOLERANCE && pnt.Y() > A.Y() - TOLERANCE)||
			(pnt.Y() > B.Y() - TOLERANCE && pnt.Y() < A.Y() + TOLERANCE))
		{
			if(fabs(A.Y() - B.Y()) < TOLERANCE)
				return ret;
			double u = (pnt.Y() - A.Y())/(B.Y()-A.Y());
			double x = GetXatU(u);
			if(x < pnt.X())
				ret.push_back(RayIntersection(u,gp_Pnt(x,pnt.Y(),0),false,false));
		}
		return ret;

	}
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

	FastArc(gp_Pnt A,gp_Pnt B, gp_Pnt C, bool cw, gp_Circ circ)
	{
		if(!cw)
		{
			gp_Pnt temp = A;
			A = B;
			B = temp;

			circ.SetAxis(circ.Axis().Reversed());
		}


		this->A = A; 
		this->B = B; 
		this->C = C; 
		this->cw = cw;
		this->m_circ = circ;
		this->m_flipped = false;



		a1 = atan2(A.Y()-C.Y(), A.X()-C.X());
		a2 = atan2(B.Y()-C.Y(), B.X()-C.X());

		if(a2<a1)
			a2+=2*Pi;

		da = a2 - a1;

		rad = C.Distance(A);

		//TODO: compute how far apart the angles are, signed!
		//da = fmod(a2-a1,2*Pi);

#ifdef CHECKFASTARC
		double tax = GetXatU(0);
		double tay = GetYatU(0);
		double tbx = GetXatU(1);
		double tby = GetYatU(1);

		if(tax != A.X() || tay != A.Y() || tbx != B.X() || tby != B.Y())
		{
			int x=0;
			x++;
		}
#endif
	}

	FastArc(){}

	gp_Circ GetCircle()
	{
		return m_circ;
	}

	void Reverse() 
	{
/*		gp_Pnt tmp = A; 
		A = B; 
		B = tmp;

		double temp = a1;
		a1 = a2;
		a2 = temp;*/

//		a1 = fmod(a1,2*Pi);
//		a2 = fmod(a2,2*Pi);

//		if(a1>a2);
//			a2+=2*Pi;

/*		da = -da; */

		m_circ.SetAxis(m_circ.Axis().Reversed());

		cw=!cw;
	}
	double GetY()
	{
		return GetY(addedAt);
	}

	double GetY(double ax)
	{
		double y = sqrt(rad*rad - (ax - C.X()) * (ax - C.X()));
		if(A.Y() - C.Y() > TOLERANCE || B.Y() - C.Y() > TOLERANCE)
			return y + C.Y();
		return C.Y() - y;
	}

	double GetU(double x, double y)
	{
		double ang = atan2(y-C.Y(),x-C.X());

		if(ang < a1-TOLERANCE)
			ang += 2*Pi;

		double u =  fmod((a2-ang)/da,2*Pi);
		if(u!=u)
		{
			int x=0;
			x++;
		}
		if(u > TOLERANCE && u < 1 - TOLERANCE)
		{
			int x=0;
			x++;
		}
#ifdef ROUNDU
		if(u > -TOLERANCE && ut < TOLERANCE)
			u = 0;
		if(u > 1 - TOLERANCE && u < 1 + TOLERANCE)
			u = 1;
#endif

		return u;
	}

	double GetXatU(double u)
	{
		double angle = a1 + u*da;
		return TransformX(rad * cos(angle) + C.X());
	}

	double GetYatU(double u)
	{
		double angle = a1 + u*da;
		return TransformY(rad * sin(angle) + C.Y());
	}

	double TransformY(double y)
	{
		return y;
	}

	double TransformX(double x)
	{
		return x;
	}

	std::vector<RayIntersection> RayIntersects(gp_Pnt p)
	{
		std::vector<RayIntersection> ret;

		double y = p.Y() - C.Y();
		if(fabs(y) > rad)
			return ret;

		double x1 = sqrt(rad*rad - y*y);
		double x2 = x1+C.X();
		x1 = C.X()-x1;
		
		if(x1 < p.X())
			ret.push_back(RayIntersection(GetU(x1,p.Y()),gp_Pnt(x1,p.Y(),0),false,false));
		if(x2 < p.X())
			ret.push_back(RayIntersection(GetU(x2,p.Y()),gp_Pnt(x2,p.Y(),0),false,false));
		return ret;
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
	void Swap()
	{
		double temp = uA;
		uA = uB;
		uB = temp;
	}	
};
