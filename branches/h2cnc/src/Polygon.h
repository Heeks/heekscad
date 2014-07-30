// Polygon.h
// Copyright (c) 2009, Perttu "celero55" Ahola
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include <list>
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

#ifndef UNITTEST_NO_HEEKS
	#include "HLine.h"
	#include "Sketch.h"
#endif

#include <gp_Pnt.hxx>
#include <gp_Lin.hxx>
#include <gp_Vec.hxx>

#define POLYGON_TOLERANCE 0.000000001

#define dabs(x) ((x)>0?(x):-(x))

enum Direction
{
	Left,
	Right
};

enum PolygonDirection
{
	PolyCW,
	PolyCCW,
	PolyUndefinedW
};

enum PolyIntersectionType
{
	NoIntersection,
	NormalIntersection,
	ParallelIntersection,
	SomeIntersection
};

class IntersectionInfo
{
public:
	bool operator < (const IntersectionInfo &info) const
	{
		return i < info.i;
	}
	std::string str() const
	{
		std::stringstream ss;
		switch(t){
		case NoIntersection:
			ss<<"NoIntersection";
			return ss.str();
		case NormalIntersection:
			ss<<"NormalIntersection";
			break;
		case ParallelIntersection:
			ss<<"ParallelIntersection";
			break;
		case SomeIntersection:
			ss<<"SomeIntersection";
			break;
		}
		ss<<"("<<p.X()<<","<<p.Y()<<")";
		return ss.str();
	}
	//Index number of the other line in the std::vector<LineSegment> parameter
	//of SweepLine
	int i;
	PolyIntersectionType t;
	gp_Pnt p;
};

const char *PDstr(PolygonDirection d);

inline bool PointsCCW(gp_Pnt a, gp_Pnt b, gp_Pnt c)
{
	return (c.Y()-a.Y())*(b.X()-a.X()) > (b.Y()-a.Y())*(c.X()-a.X());
}

/*//this would probably go like this
inline bool PointsCW(gp_Pnt a, gp_Pnt b, gp_Pnt c)
{
	return (c.Y()-a.Y())*(b.X()-a.X()) < (b.Y()-a.Y())*(c.X()-a.X());
}*/

/*inline bool IsUpperOrEqual(gp_Pnt a_left, gp_Pnt a_right, gp_Pnt b_left)
{
	//left point identical?
	if(a_left.X()==b_left.X() && a_left.Y()==b_left.Y()) return true;
	//no, check if CCW
	return PointsCCW(a_left, a_right, b_left);
}*/

class LineSegment
{
public:
	LineSegment() {}
	LineSegment(double x1, double y1, double x2, double y2)
			: a(x1,y1,0), b(x2,y2,0) {}
	LineSegment(gp_Pnt p_a, gp_Pnt p_b) : a(p_a), b(p_b) {}
	gp_Pnt endpoint(Direction d)
	{
		switch(d){
		case Left: return left();
		case Right: return right();
		}
		return b;
	}
	gp_Pnt left() const
	{
		if(a.X() < b.X()) return a;
		else if(is_vertical()) return upper();
		return b;
	}
	gp_Pnt right() const
	{
		if(a.X() > b.X()) return a;
		else if(is_vertical()) return lower();
		return b;
	}
	gp_Pnt lower() const
	{
		if(a.Y() < b.Y()) return a;
		return b;
	}
	gp_Pnt upper() const
	{
		if(a.Y() > b.Y()) return a;
		return b;
	}
	std::string str() const
	{
		std::stringstream ss;
		ss<<"{"<<m_index<<" ("<<a.X()<<", "<<a.Y()
				<<") -> ("<<b.X()<<", "<<b.Y()<<")}";
		return ss.str();
	}
	double y_value_at(double x) const
	{
		//if vertical
		if(is_vertical()) return lower().Y();
		double b1 = (right().Y() - left().Y()) / (right().X() - left().X());
		return left().Y() + b1 * (x - left().X());
		/*double b1 = (right().Y() - left().Y()) / (right().X() - left().X());
		double a1 = left().Y() - b1 * left().X();
		return a1+b1*x;*/
	}
	bool is_vertical() const
	{
		return (dabs(a.X() - b.X()) < POLYGON_TOLERANCE);
	}
	bool is_horizontal() const
	{
		return (dabs(a.Y() - b.Y()) < POLYGON_TOLERANCE);
	}
	//This is used to compare the item being added to the items already on the
	//vertical sequence.
	//Returns if this line is upper or equal than s at the immediate position
	//after left().X()
	bool IsUpperOrEqual(const LineSegment &s) const
	{
		//if this line is vertical
		if(is_vertical())
		{
			std::cout<<"LineSegment::IsUpperOrEqual(): should not happen: this="
					<<str()<<" is vertical"<<std::endl;
			return false;
			/*double b2 = (s.b.Y()-s.a.Y())/(s.b.X()-s.a.X());
			double a2 = s.a.Y()-b2*s.a.X();
			double yi = a2+b2*a.X();*/
		}
		//if the other line is vertical
		if(s.is_vertical())
		{
			std::cout<<"LineSegment::IsUpperOrEqual(): should not happen: s="
					<<s.str()<<" is vertical"<<std::endl;
			return true;
			/*double b1 = (b.Y()-a.Y())/(b.X()-a.X());
			double a1 = a.Y()-b1*a.X();
			double yi = a1+b1*s.a.X();*/
		}
		//if this line has the left point of s
		if(s.HasPoint(left()))
		{
			//the lines can't be vertical now, so it's easy
			double b1 = (right().Y()-left().Y())/(right().X()-left().X());
			double b2 = (s.right().Y()-s.left().Y())/(s.right().X()-s.left().X());
			std::cout<<"LineSegment::IsUpperOrEqual(): s has this line's left"
					" endpoint; b1="<<b1<<" b2="<<b2<<std::endl;
			return b1 >= b2 - POLYGON_TOLERANCE;
		}
		return PointsCCW(right(), left(), s.left());
	}
	//returns true if this line is upper or equal to s, just after x
	bool IsUpperOrEqualAfterX(const LineSegment &s, double x) const
	{
		//if this line is vertical
		if(is_vertical())
		{
			return false;
		}
		//if the other line is vertical
		if(s.is_vertical())
		{
			return true;
		}

		if(dabs(y_value_at(x) - s.y_value_at(x)) < POLYGON_TOLERANCE)
		{
			double b1 = (b.Y()-a.Y())/(b.X()-a.X());
			double b2 = (s.b.Y()-s.a.Y())/(s.b.X()-s.a.X());
			return b1 >= b2 - POLYGON_TOLERANCE;
		}

		return (y_value_at(x) >= s.y_value_at(x) - POLYGON_TOLERANCE);
	}
	bool HasPoint(gp_Pnt p) const
	{
		if(is_vertical()){
			//line is vertical
			double upper = a.Y() > b.Y() ? a.Y() : b.Y();
			double lower = a.Y() < b.Y() ? a.Y() : b.Y();
			if(p.Y() >= lower - POLYGON_TOLERANCE && p.Y() <= upper + POLYGON_TOLERANCE
					&& dabs(p.X() - (a.X()+b.X())/2) < POLYGON_TOLERANCE) return true;
			return false;
		}
		if(p.X() < left().X() - POLYGON_TOLERANCE
				|| p.X() > right().X() + POLYGON_TOLERANCE) return false;
		return (dabs(y_value_at(p.X()) - p.Y()) < POLYGON_TOLERANCE);
		/*double b1 = (right().Y() - left().Y()) / (right().X() - left().X());
		double y = left().Y() + b1 * (p.X() - left().X());
		return (dabs(y-p.Y()) < POLYGON_TOLERANCE);*/
	}
	bool HasPointInterior(gp_Pnt p) const
	{
		/*std::cout<<str()<<".HasPointInterior(("<<p.X()<<","<<p.Y()
				<<")"<<std::endl;*/
		if(a.IsEqual(p, POLYGON_TOLERANCE) || b.IsEqual(p, POLYGON_TOLERANCE)){
			//std::cout<<"\tis endpoint; returning false"<<std::endl;
			return false;
		}
		if(is_vertical()){
			//line is vertical
			/*std::cout<<"\tis vertical, lower()=("<<lower().X()<<","<<lower().Y()
					<<") upper="<<upper().X()<<","<<upper().Y()
					<<")"<<std::endl;*/
			if(p.Y() > lower().Y() && p.Y() < upper().Y()
					&& dabs(p.X() - (a.X()+b.X())/2) < POLYGON_TOLERANCE)
			{
				//std::cout<<"\treturning true"<<std::endl;
				return true;
			}
			//std::cout<<"\treturning false"<<std::endl;
			return false;
		}
		if((p.X() <= left().X() || p.X() >= right().X()) && !is_vertical()){
			//std::cout<<"\tpoint out of X range -> returning false"<<std::endl;
			return false;
		}
		if((p.Y() <= lower().Y() || p.Y() >= upper().Y()) && !is_horizontal()){
			//std::cout<<"\tpoint out of Y range -> returning false"<<std::endl;
			return false;
		}
		bool b = (dabs(y_value_at(p.X()) - p.Y()) < POLYGON_TOLERANCE);
		/*std::cout<<"\ty_value_at("<<p.X()<<") = "<<y_value_at(p.X())<<" -> "
				"returning "<<(b?"true":"false")<<std::endl;*/
		return b;
		/*double b1 = (right().Y() - left().Y()) / (right().X() - left().X());
		double y = left().Y() + b1 * (p.X() - left().X());
		return (dabs(y-p.Y()) < POLYGON_TOLERANCE);*/
	}
	IntersectionInfo intersects(const LineSegment &s) const
	{
		IntersectionInfo i;
		if(HasPoint(s.a) || HasPoint(s.b) || s.HasPoint(a) || s.HasPoint(b)
			|| (PointsCCW(a, s.a, s.b) != PointsCCW(b, s.a, s.b)
			&&  PointsCCW(a, b,   s.a) != PointsCCW(a, b,   s.b)))
		{
			//if one of the lines has no length, return the point
			//if(a.X() == b.X() && a.Y() == b.Y())
			if(a.IsEqual(b, POLYGON_TOLERANCE))
			{
				i.p = a;
				i.t = NormalIntersection;
			}
			//else if(s.a.X() == s.b.X() && s.a.Y() == s.b.Y())
			else if(s.a.IsEqual(s.b, POLYGON_TOLERANCE))
			{
				i.p = s.a;
				i.t = NormalIntersection;
			}
			//if both of the lines are vertical
			else if(dabs(b.X() - a.X()) < POLYGON_TOLERANCE 
					&& dabs(s.b.X() - s.a.X()) < POLYGON_TOLERANCE)
			{
				i.t = ParallelIntersection;
			}
			//if this line is vertical
			else if(dabs(b.X() - a.X()) < POLYGON_TOLERANCE)
			{
				double b2 = (s.b.Y()-s.a.Y())/(s.b.X()-s.a.X());
				double a2 = s.a.Y()-b2*s.a.X();
				double yi = a2+b2*a.X();
				i.p = gp_Pnt(a.X(), yi, 0);
				i.t = NormalIntersection;
			}
			//if the other line is vertical
			else if(dabs(s.b.X() - s.a.X()) < POLYGON_TOLERANCE)
			{
				double b1 = (b.Y()-a.Y())/(b.X()-a.X());
				double a1 = a.Y()-b1*a.X();
				double yi = a1+b1*s.a.X();
				i.p = gp_Pnt(s.a.X(), yi, 0);
				i.t = NormalIntersection;
			}
			//else
			else
			{
				double b1 = (b.Y()-a.Y())/(b.X()-a.X());
				double b2 = (s.b.Y()-s.a.Y())/(s.b.X()-s.a.X());
				if(dabs(b1 - b2) < POLYGON_TOLERANCE)
				{
					i.t = ParallelIntersection;
				}
				else
				{
					double a1 = a.Y()-b1*a.X();
					double a2 = s.a.Y()-b2*s.a.X();
					double xi = -(a1-a2)/(b1-b2);
					double yi = a1+b1*xi;
					i.p = gp_Pnt(xi, yi, 0);
					i.t = NormalIntersection;
				}
			}
		}
		else
		{
			i.t = NoIntersection;
		}
		return i;
	}
	
	//end points
	gp_Pnt a, b;

	//SweepLine inserts detected intersections into this.
	std::set<IntersectionInfo> m_intersections_set;
	//SweepLine sets this
	int m_index;
private:
};

class Endpoint
{
public:
	Endpoint(const Endpoint &e) : m_line_ptr(e.m_line_ptr), m_dir(e.m_dir)
	{}
	Endpoint(LineSegment * p_line_ptr, /*int p_line_index,*/ Direction p_dir)
			: m_line_ptr(p_line_ptr), /*m_line_index(p_line_index),*/ m_dir(p_dir)
	{}
	bool operator < (const Endpoint &e) const
	{
		if(dabs(value().X() - e.value().X()) < POLYGON_TOLERANCE && dir()==Left) return true;
		return (value().X() < e.value().X());
	}
	bool operator == (const Endpoint &e) const
	{
		return (m_dir == e.m_dir && m_line_ptr == e.m_line_ptr);
	}
	gp_Pnt value() const
	{
		if(m_dir==Left) return m_line_ptr->left();
		else return m_line_ptr->right();
	}
	bool dir() const
	{
		return m_dir != Left;
	}
	LineSegment * line_ptr() const
	{
		return m_line_ptr;
	}
	/*unsigned int line_index() const
	{
		return m_line_index;
	}*/
	std::string str() const
	{
		std::stringstream ss;
		gp_Pnt v = value();
		if(m_dir==Left) ss<<"left ("<<v.X()<<", "<<v.Y()<<")";
		else            ss<<"right ("<<v.X()<<", "<<v.Y()<<")";
		return ss.str();
	}
private:
	LineSegment * m_line_ptr;
	//unsigned int m_line_index;
	Direction m_dir;
};

class CPolygon
{
public:
	std::string name; //for use of UnionPolygons

	CPolygon()
	{
		m_dir = PolyUndefinedW;
	}
	CPolygon(std::list<gp_Pnt> &p_points)
	{
		points = p_points;
		m_dir = PolyUndefinedW;
	}
	void clear()
	{
		points.clear();
		m_dir = PolyUndefinedW;
	}
	bool empty()
	{
		return points.empty();
	}
	bool push_back(gp_Pnt point)
	{
		if(dabs(point.Z()) > 0.00000001)
		{
			//assert would be nice
			std::cout<<"EE Polygon::push_back(): "
					" dabs(point.Z()) > 0.00000001"<<std::endl;
			return false;
		}
		points.push_back(point);
		m_dir = PolyUndefinedW;
		return true;
	}
#ifndef UNITTEST_NO_HEEKS
	CSketch *MakeSketch()
	{
		if(points.size() < 2) return NULL;

		CSketch *sketch = new CSketch();
		//sketch->SetString(wxString(name.c_str(), wxConvUTF8));

		gp_Pnt oldp = *(points.begin());
		
		for(std::list<gp_Pnt>::iterator ilp = ++points.begin(); ilp != points.end(); ilp++)
		{
			if(!oldp.IsEqual(*ilp, wxGetApp().m_geom_tol))
			{
				HLine *line = new HLine(oldp, *ilp, &(wxGetApp().current_color));
				sketch->Add(line, NULL);
			}
			oldp = *ilp;
		}
		if(!oldp.IsEqual(*(points.begin()), wxGetApp().m_geom_tol))
		{
			sketch->Add(new HLine(oldp, *(points.begin()), &(wxGetApp().current_color)), NULL);
		}
		return sketch;
	}
#endif
	void Move(gp_Vec v)
	{
		for(std::list<gp_Pnt>::iterator ilp = points.begin(); ilp != points.end(); ilp++)
		{
			ilp->Translate(v);
		}
	}
	void Print() const
	{
		std::cout<<str()<<std::endl;
	}
	std::string str() const
	{
		std::stringstream stream;
		if(!name.empty()) stream<<"\""<<name<<"\"";
		stream<<"{ ";
		for(std::list<gp_Pnt>::const_iterator ilp = points.begin(); ilp != points.end(); ilp++)
		{
			stream<<"("<<ilp->X()
					<<", "<<ilp->Y()/*<<", "<<ilp->Z()*/<<")";
			if(ilp != --points.end()) stream<<", ";
		}
		stream<<" }";
		return stream.str();
	}
	std::list<gp_Pnt>::const_iterator begin() const
	{
		return points.begin();
	}
	std::list<gp_Pnt>::const_iterator end() const
	{
		return points.end();
	}
	LineSegment GetLine(std::list<gp_Pnt>::const_iterator it) const
	{
		LineSegment s;
		s.a = *it;
		it++;
		if(it==points.end()) it = points.begin();
		s.a = *it;
		return s;
	}
	void reverse()
	{
		points.reverse();
		if(m_dir==PolyCW) m_dir = PolyCCW;
		else if(m_dir==PolyCCW) m_dir = PolyCW;
		else m_dir = PolyUndefinedW;
	}

	PolygonDirection Direction();

private:
	std::list<gp_Pnt> points;
	PolygonDirection m_dir;
}; //End Polygon class definition

unsigned int SweepLine(std::vector<LineSegment> &lines_vector);

bool UnionPolygons(std::vector<LineSegment> &lines_vector,
		std::list<CPolygon> & result_list);

bool UnionPolygons(std::list<CPolygon> & polygons_list,
		std::list<CPolygon> & result_list);

bool UnionPolygons_old(std::list<CPolygon> & polygons_list,
		std::list<CPolygon> & result_list);

