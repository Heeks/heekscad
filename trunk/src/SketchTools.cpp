// SketchTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "MarkedList.h"
#include "Sketch.h"
#include "Group.h"
#include "Pad.h"
#include "Part.h"
#include "Pocket.h"
#include "ConversionTools.h"
#include "../interface/HeeksCADInterface.h"
#include "../interface/PropertyList.h"
#include "../interface/PropertyCheck.h"
#include "../interface/PropertyLength.h"
#include "HeeksConfig.h"
#include "../interface/CNCPoint.h"
#include "SketchTools.h"

extern CHeeksCADInterface heekscad_interface;

/**
	These settings relate to the fixes available in the ShapeFix_Wire class.
 */
class SketchToolOptions
{
public:
    SketchToolOptions()
    {
        HeeksConfig config;
        config.Read(_T("SketchToolOptions_reorder"), &m_reorder, true);
        config.Read(_T("SketchToolOptions_max_gap"), &m_max_gap, 0.001);
        config.Read(_T("SketchToolOptions_fix_small"), &m_fix_small, true);
        config.Read(_T("SketchToolOptions_fix_small_precision"), &m_fix_small_precision, 0.0254);
        config.Read(_T("SketchToolOptions_fix_self_intersection"), &m_fix_self_intersection, true);
        config.Read(_T("SketchToolOptions_fix_lacking"), &m_fix_lacking, true);
        config.Read(_T("SketchToolOptions_fix_degenerated"), &m_fix_degenerated, true);

        config.Read(_T("SketchToolOptions_modify_topology"), &m_modify_topology, true);
        config.Read(_T("SketchToolOptions_modify_geometry"), &m_modify_geometry, true);
        config.Read(_T("SketchToolOptions_closed_wire"), &m_closed_wire, true);
        config.Read(_T("SketchToolOptions_preference_pcurve"), &m_preference_pcurve, true);
        config.Read(_T("SketchToolOptions_fix_gaps_by_ranges"), &m_fix_gaps_by_ranges, true);

  // Standard_Integer& ModifyRemoveLoopMode() ;


    }

    void SaveSettings()
    {
        HeeksConfig config;
        config.Write(_T("SketchToolOptions_reorder"), m_reorder);
        config.Write(_T("SketchToolOptions_max_gap"), m_max_gap);
        config.Write(_T("SketchToolOptions_fix_small"), m_fix_small);
        config.Write(_T("SketchToolOptions_fix_small_precision"), m_fix_small_precision);
        config.Write(_T("SketchToolOptions_fix_self_intersection"), m_fix_self_intersection);
        config.Write(_T("SketchToolOptions_fix_lacking"), m_fix_lacking);
        config.Write(_T("SketchToolOptions_fix_degenerated"), m_fix_degenerated);
        config.Write(_T("SketchToolOptions_modify_topology"), m_modify_topology);
        config.Write(_T("SketchToolOptions_modify_geometry"), m_modify_geometry);
        config.Write(_T("SketchToolOptions_closed_wire"), m_closed_wire);
        config.Write(_T("SketchToolOptions_preference_pcurve"), m_preference_pcurve);
        config.Write(_T("SketchToolOptions_fix_gaps_by_ranges"), m_fix_gaps_by_ranges);
    }

public:
    bool m_reorder;
    double m_max_gap;
    bool m_fix_small;
    double m_fix_small_precision;
    bool m_fix_self_intersection;
    bool m_fix_lacking;
    bool m_fix_degenerated;
    bool m_modify_topology;
    bool m_modify_geometry;
    bool m_closed_wire;
    bool m_preference_pcurve;
    bool m_fix_gaps_by_ranges;
};

static SketchToolOptions sketch_tool_options;



class PadSketch:public Tool{
public:
	void Run(){
		double height = 10;
		wxGetApp().InputLength(_("Input extrusion height"), _("height"), height);
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());

		CPad::PadSketch(sketch,height);
	}
	const wxChar* GetTitle(){return _T("Pad Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pad Sketch");}
};

class PocketSketch:public Tool{
public:
	void Run(){
		double height = 10;
		wxGetApp().InputLength(_("Input pad depth"), _("height"), height);
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());

		CPocket::PocketSketch(sketch,height);
	}
	const wxChar* GetTitle(){return _T("Pocket Sketch");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Pocket Sketch");}
};


class MakeToPart:public Tool{
public:
	void Run(){
		CSketch* sketch = (CSketch*)(*wxGetApp().m_marked_list->list().begin());
		CPart* part = new CPart();
		wxGetApp().Add(part,NULL);
		sketch->Owner()->Remove(sketch);
		sketch->RemoveOwners();
		part->Add(sketch,NULL);
	}
	const wxChar* GetTitle(){return _T("Make To Part");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Make To Part");}
};

class AddToPart:public Tool{
public:
	void Run()
	{
		std::list<HeeksObj*>::const_iterator It;
		CSketch* sketch=NULL;
		CPad* pad=NULL;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* obj = (HeeksObj*)*It;
			if(obj->GetType() == SketchType)
				sketch = (CSketch*)obj;
			else
				pad = (CPad*)obj;
		}
		sketch->Owner()->Remove(sketch);
		sketch->RemoveOwners();
		pad->Add(sketch,NULL);
	}
	const wxChar* GetTitle(){return _T("Add To Part");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add To Part");}
};


class FixWire:public Tool{
public:
	void Run()
	{
		std::list<HeeksObj*>::const_iterator It;
		for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* obj = (HeeksObj*)*It;
			if(obj->GetType() == SketchType)
			{
				CSketch* sketch = (CSketch*)obj;

                std::list<TopoDS_Shape> wires;
                if (! ConvertSketchToFaceOrWire( sketch, wires, false))
                {
                    wxMessageBox(_("Failed to convert sketch to wire"));
                } // End if - then
                else
                {
                    try {
                        // For all wires in this sketch...
                        for(std::list<TopoDS_Shape>::iterator It2 = wires.begin(); It2 != wires.end(); It2++)
                        {
                            TopoDS_Shape& wire_to_fix = *It2;
                            ShapeFix_Wire fix;
                            fix.Load( TopoDS::Wire(wire_to_fix) );
                            // fix.ClearModes();

                            fix.ModifyTopologyMode() = sketch_tool_options.m_modify_topology;
                            fix.ModifyGeometryMode() = sketch_tool_options.m_modify_geometry;
                            fix.ClosedWireMode() = sketch_tool_options.m_closed_wire;
                            fix.PreferencePCurveMode() = sketch_tool_options.m_preference_pcurve;
                            fix.FixGapsByRangesMode() = sketch_tool_options.m_fix_gaps_by_ranges;
                            fix.FixSelfIntersectionMode() = sketch_tool_options.m_fix_self_intersection;
                            fix.FixReorderMode() = sketch_tool_options.m_reorder;
                            fix.FixDegeneratedMode() = sketch_tool_options.m_fix_degenerated;

                            if (sketch_tool_options.m_fix_small)
                            {
                                fix.FixSmallMode() = true;
                                fix.FixSmall( false, sketch_tool_options.m_fix_small_precision );
                            }

                            if (sketch_tool_options.m_fix_lacking)
                            {
                                fix.FixLackingMode() = sketch_tool_options.m_fix_lacking;
                                fix.FixLacking(false);
                            }

                            if (! fix.Perform())
                            {
                                wxMessageBox(_("Wire fix failed"));
                            }
                            else
                            {
                                TopoDS_Shape wire = fix.Wire();
                                HeeksObj *new_sketch = heekscad_interface.NewSketch();
                                double deviation = 0.001;
                                if (!  ConvertWireToSketch(TopoDS::Wire(wire), new_sketch, deviation))
                                {
                                    wxMessageBox(_("Failed to convert wire to sketch"));
                                }
                                else
                                {
                                    if (sketch != NULL)
                                    {
                                        heekscad_interface.Remove( sketch );
                                        sketch = NULL;
                                    }

                                    heekscad_interface.Add(new_sketch, NULL);
                                }
                            }
                        }
                    } // End try
                    catch (Standard_Failure & error) {
                        (void) error;	// Avoid the compiler warning.
                        Handle_Standard_Failure e = Standard_Failure::Caught();
                        wxMessageBox(_("Failed to fix wire"));
                    } // End catch
                }
			}
		}
	}
	const wxChar* GetTitle(){return _T("Fix Wire");}
	wxString BitmapPath(){return _T("new");}
	const wxChar* GetToolTip(){return _("Add To Part");}
};



static PadSketch pad_sketch;
static MakeToPart make_to_part;
static AddToPart add_to_part;
static PocketSketch pocket_sketch;
static FixWire fix_wire;
static SimplifySketchTool simplify_sketch_tool;


void GetSketchMenuTools(std::list<Tool*>* t_list){
	int count=0;
	bool gotsketch=false;
	bool gotpart=false;
	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType)
			gotsketch=true;
		if(object->GetType() == PartType)
			gotpart=true;
		count++;
	}

	if(count == 2 && gotsketch && gotpart)
		t_list->push_back(&add_to_part);

	if(count!=1 || !gotsketch)
		return;

	t_list->push_back(&pad_sketch);
	t_list->push_back(&pocket_sketch);
	t_list->push_back(&make_to_part);
	// t_list->push_back(&fix_wire);    /* This is not ready yet */
	t_list->push_back(&simplify_sketch_tool);
}

void on_set_sketchtool_option(bool value, HeeksObj* object){
	*((bool *) object) = value;
	sketch_tool_options.SaveSettings();
}

void on_set_sketchtool_option(double value, HeeksObj* object){
	*((double *) object) = value;
	sketch_tool_options.SaveSettings();
}

void SketchTools_GetOptions(std::list<Property *> *list)
{
    return; // This is not ready yet.

    PropertyList* sketchtools = new PropertyList(_("fix wire options"));
	sketchtools->m_list.push_back(new PropertyCheck(_("reorder"), sketch_tool_options.m_reorder, (HeeksObj *) (&sketch_tool_options.m_reorder), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix small"), sketch_tool_options.m_fix_small, (HeeksObj *) (&sketch_tool_options.m_fix_small), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyLength(_("fix small precision"), sketch_tool_options.m_fix_small_precision, (HeeksObj *) (&sketch_tool_options.m_fix_small_precision), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix self intersection"), sketch_tool_options.m_fix_self_intersection, (HeeksObj *) (&sketch_tool_options.m_fix_self_intersection), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix lacking"), sketch_tool_options.m_fix_lacking, (HeeksObj *) (&sketch_tool_options.m_fix_lacking), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix degenerated"), sketch_tool_options.m_fix_degenerated, (HeeksObj *) (&sketch_tool_options.m_fix_degenerated), on_set_sketchtool_option));

	sketchtools->m_list.push_back(new PropertyCheck(_("modify topology"), sketch_tool_options.m_modify_topology, (HeeksObj *) (&sketch_tool_options.m_modify_topology), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("modify geometry"), sketch_tool_options.m_modify_geometry, (HeeksObj *) (&sketch_tool_options.m_modify_geometry), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("closed wire"), sketch_tool_options.m_closed_wire, (HeeksObj *) (&sketch_tool_options.m_closed_wire), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("preference pcurve"), sketch_tool_options.m_preference_pcurve, (HeeksObj *) (&sketch_tool_options.m_preference_pcurve), on_set_sketchtool_option));
	sketchtools->m_list.push_back(new PropertyCheck(_("fix gaps by ranges"), sketch_tool_options.m_fix_gaps_by_ranges, (HeeksObj *) (&sketch_tool_options.m_fix_gaps_by_ranges), on_set_sketchtool_option));

	list->push_back(sketchtools);
}


/* static */ gp_Pnt SimplifySketchTool::GetStart(const TopoDS_Edge &edge)
{
    BRepAdaptor_Curve curve(edge);
    double uStart = curve.FirstParameter();
    gp_Pnt PS;
    gp_Vec VS;
    curve.D1(uStart, PS, VS);

    return(PS);
}

/* static */ gp_Pnt SimplifySketchTool::GetEnd(const TopoDS_Edge &edge)
{
    BRepAdaptor_Curve curve(edge);
    double uEnd = curve.LastParameter();
    gp_Pnt PE;
    gp_Vec VE;
    curve.D1(uEnd, PE, VE);

    return(PE);
}


struct EdgeComparison : public std::binary_function<const TopoDS_Edge &, const TopoDS_Edge &, bool >
{
    EdgeComparison( const TopoDS_Edge & edge )
    {
        m_reference_edge = edge;
    }

    bool operator()( const TopoDS_Edge & lhs, const TopoDS_Edge & rhs ) const
    {

        std::vector<double> lhs_distances;
        lhs_distances.push_back( SimplifySketchTool::GetStart(m_reference_edge).Distance( SimplifySketchTool::GetStart(lhs) ) );
        lhs_distances.push_back( SimplifySketchTool::GetStart(m_reference_edge).Distance( SimplifySketchTool::GetEnd(lhs) ) );
        lhs_distances.push_back( SimplifySketchTool::GetEnd(m_reference_edge).Distance( SimplifySketchTool::GetStart(lhs) ) );
        lhs_distances.push_back( SimplifySketchTool::GetEnd(m_reference_edge).Distance( SimplifySketchTool::GetEnd(lhs) ) );
        std::sort(lhs_distances.begin(), lhs_distances.end());

        std::vector<double> rhs_distances;
        rhs_distances.push_back( SimplifySketchTool::GetStart(m_reference_edge).Distance( SimplifySketchTool::GetStart(rhs) ) );
        rhs_distances.push_back( SimplifySketchTool::GetStart(m_reference_edge).Distance( SimplifySketchTool::GetEnd(rhs) ) );
        rhs_distances.push_back( SimplifySketchTool::GetEnd(m_reference_edge).Distance( SimplifySketchTool::GetStart(rhs) ) );
        rhs_distances.push_back( SimplifySketchTool::GetEnd(m_reference_edge).Distance( SimplifySketchTool::GetEnd(rhs) ) );
        std::sort(rhs_distances.begin(), rhs_distances.end());

        return(*(lhs_distances.begin()) < *(rhs_distances.begin()));
    }

    TopoDS_Edge m_reference_edge;
};

/* static */ std::vector<TopoDS_Edge> SimplifySketchTool::SortEdges( const TopoDS_Wire & wire )
{
    std::vector<TopoDS_Edge> edges;

	for(BRepTools_WireExplorer expEdge(TopoDS::Wire(wire)); expEdge.More(); expEdge.Next())
	{
	    edges.push_back( TopoDS_Edge(expEdge.Current()) );
	} // End for

	for (std::vector<TopoDS_Edge>::iterator l_itEdge = edges.begin(); l_itEdge != edges.end(); l_itEdge++)
    {
        if (l_itEdge == edges.begin())
        {
            // It's the first edge.  Find the edge whose endpoint is closest to gp_Pnt(0,0,0) so that
            // the resutls of this sorting are consistent.  When we just use the first edge in the
            // wire, we end up with different results every time.  We want consistency so that, if we
            // use this Contour operation as a location for drilling a relief hole (one day), we want
            // to be sure the machining will begin from a consistently known location.

            std::vector<TopoDS_Edge>::iterator l_itStartingEdge = edges.begin();
            gp_Pnt closest_point = GetStart(*l_itStartingEdge);
            if (GetEnd(*l_itStartingEdge).Distance(gp_Pnt(0,0,0)) < closest_point.Distance(gp_Pnt(0,0,0)))
            {
                closest_point = GetEnd(*l_itStartingEdge);
            }
            for (std::vector<TopoDS_Edge>::iterator l_itCheck = edges.begin(); l_itCheck != edges.end(); l_itCheck++)
            {
                if (GetStart(*l_itCheck).Distance(gp_Pnt(0,0,0)) < closest_point.Distance(gp_Pnt(0,0,0)))
                {
                    closest_point = GetStart(*l_itCheck);
                    l_itStartingEdge = l_itCheck;
                }

                if (GetEnd(*l_itCheck).Distance(gp_Pnt(0,0,0)) < closest_point.Distance(gp_Pnt(0,0,0)))
                {
                    closest_point = GetEnd(*l_itCheck);
                    l_itStartingEdge = l_itCheck;
                }
            }

            EdgeComparison compare( *l_itStartingEdge );
            std::sort( edges.begin(), edges.end(), compare );
        } // End if - then
        else
        {
            // We've already begun.  Just sort based on the previous point's location.
            std::vector<TopoDS_Edge>::iterator l_itNextEdge = l_itEdge;
            l_itNextEdge++;

            if (l_itNextEdge != edges.end())
            {
                EdgeComparison compare( *l_itEdge );
                std::sort( l_itNextEdge, edges.end(), compare );
            } // End if - then
        } // End if - else
    } // End for

    return(edges);

} // End SortEdges() method


/**
    When we're starting a new sequence of edges, we want to run along the first edge
    so that we end up nearby to the next edge in the sorted sequence.  If we go in the
    wrong direction then we're just going to have to rapid up to clearance height and
    move to the beginning of the next edge anyway.  This routine returns 'true' if
    the next edge is closer to the 'end' of this edge and 'false' if it's closer to
    the 'beginning' of this edge.  This tell us whether we want to run forwards
    or backwards along this edge so that we're setup ready to machine the next edge.
 */
/* static */ bool SimplifySketchTool::DirectionTowarardsNextEdge( const TopoDS_Edge &from, const TopoDS_Edge &to )
{
    const bool forwards = true;
    const bool backwards = false;

    bool direction = forwards;

    double min_distance = 9999999;  // Some big number.
    if (GetStart(from).Distance( GetEnd( to )) < min_distance)
    {
        min_distance = GetStart( from ).Distance( GetEnd( to ));
        direction = backwards;
    }

    if (GetEnd(from).Distance( GetEnd( to )) < min_distance)
    {
        min_distance = GetEnd(from).Distance( GetEnd( to ));
        direction = forwards;
    }

    if (GetStart(from).Distance( GetStart( to )) < min_distance)
    {
        min_distance = GetStart(from).Distance( GetStart( to ));
        direction = backwards;
    }

    if (GetEnd(from).Distance( GetStart( to )) < min_distance)
    {
        min_distance = GetEnd(from).Distance( GetStart( to ));
        direction = forwards;
    }

    return(direction);
}


SimplifySketchTool::SimplifySketchTool()
{
	m_object = NULL;
	// m_deviation = 0.0254;		// one thousanth of an inch
	m_deviation = 1.0;
}

SimplifySketchTool::~SimplifySketchTool(void)
{
}


std::list<CNCPoint> SimplifySketchTool::GetPoints( TopoDS_Wire wire, const double deviation )
{
	std::list<CNCPoint> points;

	std::vector<TopoDS_Edge> edges = SortEdges(wire);
	CNCPoint last_position(0.0, 0.0, 0.0);

    for (std::vector<TopoDS_Edge>::size_type i=0; i<edges.size(); i++)
	{
		const TopoDS_Shape &E = edges[i];

		// enum GeomAbs_CurveType
		// 0 - GeomAbs_Line
		// 1 - GeomAbs_Circle
		// 2 - GeomAbs_Ellipse
		// 3 - GeomAbs_Hyperbola
		// 4 - GeomAbs_Parabola
		// 5 - GeomAbs_BezierCurve
		// 6 - GeomAbs_BSplineCurve
		// 7 - GeomAbs_OtherCurve

		BRepAdaptor_Curve curve(TopoDS::Edge(E));
		GeomAbs_CurveType curve_type = curve.GetType();

		switch(curve_type)
		{
			case GeomAbs_Line:
				// make a line
			{
				double uStart = curve.FirstParameter();
				double uEnd = curve.LastParameter();
				gp_Pnt PS;
				gp_Vec VS;
				curve.D1(uStart, PS, VS);
				gp_Pnt PE;
				gp_Vec VE;
				curve.D1(uEnd, PE, VE);

				if (last_position == CNCPoint(PS))
				{
					// We're heading towards the PE point.
					CNCPoint point(PE);
					points.push_back(point);
					last_position = point;
				} // End if - then
				else if (last_position == CNCPoint(PE))
				{
					CNCPoint point(PS);
					points.push_back(point);
					last_position = point;
				}
				else
				{
					CNCPoint start(PS);
					CNCPoint end(PE);

					if (i < (edges.size()-1))
					{
                        if (! DirectionTowarardsNextEdge( edges[i], edges[i+1] ))
                        {
                            // The next edge is closer to this edge's start point.  reverse direction
                            // so that the next movement is better.

                            CNCPoint temp = start;
                            start = end;
                            end = temp;
                        }
					}

					points.push_back(start);
					points.push_back(end);
					last_position = end;
				}
			}
			break;

            
			default:
			{
				// make lots of small lines
				double uStart = curve.FirstParameter();
				double uEnd = curve.LastParameter();
				gp_Pnt PS;
				gp_Vec VS;
				curve.D1(uStart, PS, VS);
				gp_Pnt PE;
				gp_Vec VE;
				curve.D1(uEnd, PE, VE);

				TopoDS_Edge edge(TopoDS::Edge(E));
				BRepTools::Clean(edge);
				BRepMesh::Mesh(edge, deviation);

				TopLoc_Location L;
				Handle(Poly_Polygon3D) Polyg = BRep_Tool::Polygon3D(edge, L);
				if (!Polyg.IsNull()) {
					const TColgp_Array1OfPnt& Points = Polyg->Nodes();
					Standard_Integer po;
					int i = 0;
					std::list<CNCPoint> interpolated_points;
					for (po = Points.Lower(); po <= Points.Upper(); po++, i++) {
						CNCPoint p = (Points.Value(po)).Transformed(L);
						interpolated_points.push_back(p);
					} // End for

					// See if we should go from the start to the end or the end to the start.
					if (*interpolated_points.rbegin() == last_position)
					{
						// We need to go from the end to the start.  Reverse the point locations to
						// make this easier.

						interpolated_points.reverse();
					} // End if - then

					if (*interpolated_points.begin() != last_position)
					{
						// This curve is not nearby to the last_position.  Rapid to the start
						// point to start this off.

						// We need to move to the start BEFORE machining this line.
						CNCPoint start(last_position);
						CNCPoint end(*interpolated_points.begin());

						points.push_back(end);
						last_position = end;
					}

					for (std::list<CNCPoint>::iterator itPoint = interpolated_points.begin(); itPoint != interpolated_points.end(); itPoint++)
					{
						if (*itPoint != last_position)
						{
							points.push_back(*itPoint);
							last_position = *itPoint;
						} // End if - then
					} // End for
				} // End if - then
			}
			break;
		} // End switch
	}

	return(points);
}

void SimplifySketchTool::Run()
{
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		if (object->GetType() == SketchType)
		{
			std::list<TopoDS_Shape> wires;
			heekscad_interface.ConvertSketchToFaceOrWire(object, wires, false);
			for (std::list<TopoDS_Shape>::iterator itWire = wires.begin(); itWire != wires.end(); itWire++)
			{
				std::list<CNCPoint> points = GetPoints( TopoDS::Wire(*itWire), m_deviation );
				// printf("We got %d points\n", points.size());

				// Now keep removing points from this list as long as the midpoints are within deviation of
				// the line between the two neighbour points.

				bool points_removed = false;
				do {
					points_removed = false;

					for (std::list<CNCPoint>::iterator itPoint = points.begin(); itPoint != points.end(); itPoint++ )
					{
						std::list<CNCPoint>::iterator itP1 = itPoint;
						std::list<CNCPoint>::iterator itP2 = itPoint;
						std::list<CNCPoint>::iterator itP3 = itPoint;

						itP2++;
						if (itP2 != points.end())
						{
							itP3 = itP2;
							itP3++;

							if (itP3 != points.end())
							{
								// First see if p1 and p2 are too close to each other.
								if (itP1->Distance(*itP2) < m_deviation)
								{
									// Discard p2.
									points.erase(itP2);
									points_removed = true;
									continue;
								}

								if (itP2->Distance(*itP3) < m_deviation)
								{
									// Discard p2
									points.erase(itP2);
									points_removed = true;
									continue;
								}

								// Now draw a line between p1 and p3.  Measure the distance between p2 and the nearest point
								// along that line.  If this distance is less than the max deviation then discard p2.
								

							}
						}
					} // End for

				} while (points_removed == true);

				if (points.size() >= 2)
				{
					HeeksObj *sketch = heekscad_interface.NewSketch();
					for (std::list<CNCPoint>::iterator itPoint = points.begin(); itPoint != points.end(); itPoint++)
					{
						std::list<CNCPoint>::iterator itNext = itPoint;
						itNext++;
						if (itNext == points.end()) continue;

						double start[3], end[3];
						itPoint->ToDoubleArray(start);
						itNext->ToDoubleArray(end);

						sketch->Add(heekscad_interface.NewLine(start, end), NULL);
					} // End for

					heekscad_interface.Add(sketch, NULL);
				}

				// printf("Now we have %d points\n", points.size());
			} // End for
		} // End if - then

		

	} // End for

} // End Run() method
