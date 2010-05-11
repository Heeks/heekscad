// CNCPoint.h
/*
 * Copyright (c) 2009, Dan Heeks, Perttu Ahola
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#pragma once

#include <list>
#include <vector>

#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"


/**
	This is simply a wrapper around the gp_Pnt class from the OpenCascade library
	that allows objects of this class to be used with methods such as std::sort() etc.
 */
class CNCPoint : public gp_Pnt {
public:
	CNCPoint();
	CNCPoint( const double *xyz );
	CNCPoint( const double &x, const double &y, const double &z );
	CNCPoint( const gp_Pnt & rhs );

	double X(const bool in_drawing_units = false) const;
	double Y(const bool in_drawing_units = false) const;
	double Z(const bool in_drawing_units = false) const;

	CNCPoint & operator+= ( const CNCPoint & rhs );
	CNCPoint operator- ( const CNCPoint & rhs ) const;

	bool operator==( const CNCPoint & rhs ) const;
	bool operator!=( const CNCPoint & rhs ) const;
	bool operator<( const CNCPoint & rhs ) const;

	void ToDoubleArray( double *pArrayOfThree ) const;

private:
	double Tolerance() const;
	double Units() const;
}; // End CNCPoint class definition.



/**
	By defining a structure that inherits from std::binary_function and has an operator() method, we
	can use this class to sort lists or vectors of CNCPoint objects.  We will do this, initially, to
	sort points of NC operations so as to minimize rapid travels.

	The example code to call this would be;
	    std::vector<CNCPoint> points;		// Some container of CNCPoint objects
		points.push_back(CNCPoint(3,4,5));	// Populate it with good data
		points.push_back(CNCPoint(6,7,8));

		for (std::vector<CNCPoint>::iterator l_itPoint = points.begin(); l_itPoint != points.end(); l_itPoint++)
		{
			std::vector<CNCPoint>::iterator l_itNextPoint = l_itPoint;
			l_itNextPoint++;

			if (l_itNextPoint != points.end())
			{
				sort_points_by_distance compare( *l_itPoint );
				std::sort( l_itNextPoint, points.end(), compare );
			} // End if - then
		} // End for
 */
struct sort_points_by_distance : public std::binary_function< const CNCPoint &, const CNCPoint &, bool >
{
	sort_points_by_distance( const CNCPoint & reference_point )
	{
		m_reference_point = reference_point;
	} // End constructor

	CNCPoint m_reference_point;

	// Return true if dist(lhs to ref) < dist(rhs to ref)
	bool operator()( const CNCPoint & lhs, const CNCPoint & rhs ) const
	{
		return( lhs.Distance( m_reference_point ) < rhs.Distance( m_reference_point ) );
	} // End operator() overload
}; // End sort_points_by_distance structure definition.



struct sort_points_by_z : public std::binary_function< const CNCPoint &, const CNCPoint &, bool >
{
	bool operator()( const CNCPoint & lhs, const CNCPoint & rhs ) const
	{
		return( lhs.Z() < rhs.Z() );
	} // End operator() overload
}; // End sort_points_by_z structure definition.



/**
	This is simply a wrapper around the gp_Pnt class from the OpenCascade library
	that allows objects of this class to be used with methods such as std::sort() etc.
 */
class CNCVector : public gp_Vec {
public:
	CNCVector();
	CNCVector( const double *xyz );
	CNCVector( const double &x, const double &y, const double &z );
	CNCVector( const gp_Vec & rhs );

	bool operator==( const CNCVector & rhs ) const;
	bool operator!=( const CNCVector & rhs ) const;
	bool operator<( const CNCVector & rhs ) const;
private:
	double Tolerance() const;
}; // End CNCVector class definition.






