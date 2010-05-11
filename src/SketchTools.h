// SketchTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Tool.h"
#include "../interface/CNCPoint.h"

#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

void GetSketchMenuTools(std::list<Tool*>* t_list);

class SimplifySketchTool: public Tool{
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


