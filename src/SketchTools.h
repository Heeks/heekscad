// SketchTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Tool.h"

#include <list>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "gp_Pnt.hxx"


void GetSketchMenuTools(std::list<Tool*>* t_list);

class SimplifySketchTool: public Tool{
    public:
        /**
            This is simply a wrapper around the gp_Pnt class from the OpenCascade library
            that allows objects of this class to be used with methods such as std::sort() etc.
         */
        class SortPoint : public gp_Pnt {
        public:
            SortPoint();
            SortPoint( const double *xyz );
            SortPoint( const double &x, const double &y, const double &z );
            SortPoint( const gp_Pnt & rhs );

            SortPoint & operator+= ( const SortPoint & rhs );
            SortPoint operator- ( const SortPoint & rhs ) const;

            bool operator==( const SortPoint & rhs ) const;
            bool operator!=( const SortPoint & rhs ) const;
            bool operator<( const SortPoint & rhs ) const;

            void ToDoubleArray( double *pArrayOfThree ) const;

        private:
            double Tolerance() const;
        }; // End SortPoint class definition.

		struct sort_points_by_distance : public std::binary_function< const SortPoint &, const SortPoint &, bool >
		{
			sort_points_by_distance( const SortPoint & reference_point )
			{
				m_reference_point = reference_point;
			} // End constructor

			SortPoint m_reference_point;

			// Return true if dist(lhs to ref) < dist(rhs to ref)
			bool operator()( const SortPoint & lhs, const SortPoint & rhs ) const
			{
				return( lhs.Distance( m_reference_point ) < rhs.Distance( m_reference_point ) );
			} // End operator() overload
		}; // End sort_points_by_distance structure definition.

protected:
	HeeksObj *m_object;
	double	m_deviation;

public:
	static std::list<SortPoint> GetPoints( TopoDS_Wire wire, const double deviation );

public:
	static gp_Pnt GetStart(const TopoDS_Edge &edge);
	static gp_Pnt GetEnd(const TopoDS_Edge &edge);
	static std::vector<TopoDS_Edge> SortEdges( const TopoDS_Wire & wire );

	/**
		When we're starting a new sequence of edges, we want to run along the first edge
		so that we end up nearby to the next edge in the sorted sequence.  If we go in the
		wrong direction then we're just going to have to rapid up to clearance height and
		move to the beginning of the next edge anyway.  This routine returns 'true' if
		the next edge is closer to the 'end' of this edge and 'false' if it's closer to
		the 'beginning' of this edge.  This tell us whether we want to run forwards
		or backwards along this edge so that we're setup ready to machine the next edge.
	 */
	static bool DirectionTowarardsNextEdge( const TopoDS_Edge &from, const TopoDS_Edge &to );


public:
	SimplifySketchTool();
	~SimplifySketchTool(void);

	// Tool's virtual functions
    virtual void Run();
	virtual const wxChar* GetTitle(){return _("Simplify Sketch to Lines");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pad Sketch");}
};

class SimplifySketchToBSplines: public SimplifySketchTool {
public:

    virtual void Run();
	const wxChar* GetTitle(){return _("Simplify Sketch to BSplines");}
};


