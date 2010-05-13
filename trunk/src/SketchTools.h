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
    private:
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

            CNCPoint & operator+= ( const CNCPoint & rhs );
            CNCPoint operator- ( const CNCPoint & rhs ) const;

            bool operator==( const CNCPoint & rhs ) const;
            bool operator!=( const CNCPoint & rhs ) const;
            bool operator<( const CNCPoint & rhs ) const;

            void ToDoubleArray( double *pArrayOfThree ) const;

        private:
            double Tolerance() const;
        }; // End CNCPoint class definition.

private:
	HeeksObj *m_object;
	double	m_deviation;

private:
	std::list<CNCPoint> GetPoints( TopoDS_Wire wire, const double deviation );

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
	void Run();
	const wxChar* GetTitle(){return _T("Simplify Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pad Sketch");}
};


