// ConversionTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "ConversionTools.h"
#include "MarkedList.h"
#include "HLine.h"
#include "HArc.h"
#include "HEllipse.h"
#include "HCircle.h"
#include "HSpline.h"
#include "Wire.h"
#include "Face.h"
#include "Edge.h"
#include "Shape.h"
#include "Sketch.h"
#include "Group.h"
#include "MultiPoly.h"
#include "Polygon.h"

void GetConversionMenuTools(std::list<Tool*>* t_list){
	// Tools for multiple selected items.
	bool lines_or_arcs_etc_in_marked_list = false;
	int sketches_in_marked_list = 0;
	bool group_in_marked_list = false;
	bool edges_in_marked_list = false;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
			case ArcType:
			case CircleType:
			case EllipseType:
			case SplineType:
				lines_or_arcs_etc_in_marked_list = true;
				break;
			case SketchType:
				sketches_in_marked_list++;
				break;
			case GroupType:
				group_in_marked_list = true;
				break;
			case EdgeType:
				edges_in_marked_list = true;
		}
	}

	if(lines_or_arcs_etc_in_marked_list)
	{
		t_list->push_back(new MakeLineArcsToSketch);
	}

	if(sketches_in_marked_list > 1){
		t_list->push_back(new ConvertSketchesToFace);
		t_list->push_back(new SketchesArcsToLines);
		t_list->push_back(new CombineSketches);
#ifdef UNITE_SKETCHES
		t_list->push_back(new UniteSketches);
#endif
	}

	if(wxGetApp().m_marked_list->list().size() > 1)t_list->push_back(new GroupSelected);
	if(group_in_marked_list)t_list->push_back(new UngroupSelected);
	if(edges_in_marked_list)t_list->push_back(new MakeEdgesToSketch);
}

bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire &wire)
{
	std::list<TopoDS_Edge> edges;
	std::list<HeeksObj*> list2;
	std::list<HeeksObj*>::const_iterator It;
	for(It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{
				list2.push_back(child);
			}
		}
		else{
			list2.push_back(object);
		}
	}

	for(std::list<HeeksObj*>::iterator It = list2.begin(); It != list2.end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
				{
					HLine* line = (HLine*)object;
					edges.push_back(BRepBuilderAPI_MakeEdge(line->A->m_p, line->B->m_p));
				}
				break;
			case ArcType:
				{
					HArc* arc = (HArc*)object;
					edges.push_back(BRepBuilderAPI_MakeEdge(arc->GetCircle(), arc->A->m_p, arc->B->m_p));
				}
				break;
		}
	}

	if(edges.size() > 0){
		BRepBuilderAPI_MakeWire wire_maker;
		std::list<TopoDS_Edge>::iterator It;
		for(It = edges.begin(); It != edges.end(); It++)
		{
			TopoDS_Edge &edge = *It;
			wire_maker.Add(edge);
		}

		wire = wire_maker.Wire();
		return true;
	}

	return false;
}

bool ConvertSketchToFaceOrWire(HeeksObj* object, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire)
{
	if(object->GetType() != SketchType && object->GetType() != CircleType)return false;
	std::list<HeeksObj*> line_arc_list;

	if(object->GetType() == SketchType)
	{
		std::list<HeeksObj*> new_separate_sketches;
		((CSketch*)object)->ExtractSeparateSketches(new_separate_sketches);
		if(new_separate_sketches.size() > 1)
		{
			// call again with each separate sketch
			for(std::list<HeeksObj*>::iterator It = new_separate_sketches.begin(); It != new_separate_sketches.end(); It++)
			{
				HeeksObj* sketch = *It;
				if(!ConvertSketchToFaceOrWire(sketch, face_or_wire, face_not_wire))return false;
			}
			return true;
		}
		else
		{
			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{
				line_arc_list.push_back(child);
			}
		}
	}
	else
	{
		line_arc_list.push_back(object);
	}

	std::list<TopoDS_Edge> edges;
	for(std::list<HeeksObj*>::const_iterator It = line_arc_list.begin(); It != line_arc_list.end(); It++){
	    try {
            HeeksObj* object = *It;
            switch(object->GetType()){
                case LineType:
                    {
                        HLine* line = (HLine*)object;
                        if(!(line->A->m_p.IsEqual(line->B->m_p, wxGetApp().m_geom_tol)))
                        {
                            edges.push_back(BRepBuilderAPI_MakeEdge(line->A->m_p, line->B->m_p));
                        }
                    }
                    break;
                case ArcType:
                    {
                        HArc* arc = (HArc*)object;

                        BRepBuilderAPI_MakeEdge edge(arc->GetCircle(), arc->A->m_p, arc->B->m_p);
                        if (! edge.IsDone())
                        {
							/*
                            BRepBuilderAPI_EdgeError error = edge.Error();
                            switch(error)
                            {
                                case  BRepBuilderAPI_EdgeDone:
                                break;

                                case BRepBuilderAPI_PointProjectionFailed:
                                wxMessageBox(wxString(_("BRepBuilderAPI_PointProjectionFailed")) );
                                break;

                                case BRepBuilderAPI_ParameterOutOfRange:
                                wxMessageBox(wxString(_("BRepBuilderAPI_ParameterOutOfRange")) );
                                break;

                                case BRepBuilderAPI_DifferentPointsOnClosedCurve:
                                wxMessageBox(wxString(_("BRepBuilderAPI_DifferentPointsOnClosedCurve")) );
                                break;

                                case BRepBuilderAPI_PointWithInfiniteParameter:
                                wxMessageBox(wxString(_("BRepBuilderAPI_PointWithInfiniteParameter")) );
                                break;

                                case BRepBuilderAPI_DifferentsPointAndParameter:
                                wxMessageBox(wxString(_("BRepBuilderAPI_DifferentsPointAndParameter")) );
                                break;

                                case BRepBuilderAPI_LineThroughIdenticPoints:
                                wxMessageBox(wxString(_("BRepBuilderAPI_LineThroughIdenticPoints")) );
                                break;

                                default:
                                wxMessageBox(wxString(_("Unknown error")) );
                                break;
                            }
							*/

							return(false);
                        }
                        else
                        {
                            edges.push_back(edge.Edge());
                        }
                    }
                    break;
                case CircleType:
                    {
                        HCircle* circle = (HCircle*)object;
                        edges.push_back(BRepBuilderAPI_MakeEdge(circle->GetCircle()));
                    }
                    break;
                case EllipseType:
                    {
                        HEllipse* ellipse = (HEllipse*)object;
                        edges.push_back(BRepBuilderAPI_MakeEdge(ellipse->GetEllipse()));
                    }
                    break;
                case SplineType:
                    {
                        HSpline* spline = (HSpline*)object;
                        edges.push_back(BRepBuilderAPI_MakeEdge(spline->m_spline));
                    }
                    break;
            }
	    } // End try
	    catch(Standard_Failure)
	    {
	        Handle_Standard_Failure e = Standard_Failure::Caught();
			wxMessageBox(wxString(_("Error converting sketch to face")) + _T(": ") + Ctt(e->GetMessageString()));
			return false;
	    }
	}

	if(edges.size() > 0){
		try
		{
			BRepBuilderAPI_MakeWire wire_maker;
			std::list<TopoDS_Edge>::iterator It;
			for(It = edges.begin(); It != edges.end(); It++)
			{
				TopoDS_Edge &edge = *It;
				wire_maker.Add(edge);
			}

			if(face_not_wire)
			{
				face_or_wire.push_back(BRepBuilderAPI_MakeFace(wire_maker.Wire()));
			}
			else
			{
				face_or_wire.push_back(wire_maker.Wire());
			}
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			wxMessageBox(wxString(_("Error converting sketch to face")) + _T(": ") + Ctt(e->GetMessageString()));
			return false;
		}
		catch(...)
		{
			wxMessageBox(_("Fatal Error converting sketch to face"));
			return false;
		}
		return true;
	}

	return false;
}

bool ConvertFaceToSketch2(const TopoDS_Face& face, HeeksObj* sketch, double deviation)
{
	// given a face, this adds lines and arcs to the given sketch
	// loop through all the loops
	TopoDS_Wire outerWire=BRepTools::OuterWire(face);

	for (TopExp_Explorer expWire(face, TopAbs_WIRE); expWire.More(); expWire.Next())
	{
		const TopoDS_Shape &W = expWire.Current();
		for(BRepTools_WireExplorer expEdge(TopoDS::Wire(W)); expEdge.More(); expEdge.Next())
		{
			const TopoDS_Shape &E = expEdge.Current();
			if(!ConvertEdgeToSketch2(TopoDS::Edge(E), sketch, deviation))return false;
		}
	}

	return true; // success
}


bool ConvertWireToSketch(const TopoDS_Wire& wire, HeeksObj* sketch, double deviation)
{
    const TopoDS_Shape &W = wire;
    for(BRepTools_WireExplorer expEdge(TopoDS::Wire(W)); expEdge.More(); expEdge.Next())
    {
        const TopoDS_Shape &E = expEdge.Current();
        if(!ConvertEdgeToSketch2(TopoDS::Edge(E), sketch, deviation))return false;
    }

	return true; // success
}



bool ConvertEdgeToSketch2(const TopoDS_Edge& edge, HeeksObj* sketch, double deviation)
{
	// enum GeomAbs_CurveType
	// 0 - GeomAbs_Line
	// 1 - GeomAbs_Circle
	// 2 - GeomAbs_Ellipse
	// 3 - GeomAbs_Hyperbola
	// 4 - GeomAbs_Parabola
	// 5 - GeomAbs_BezierCurve
	// 6 - GeomAbs_BSplineCurve
	// 7 - GeomAbs_OtherCurve

	BRepAdaptor_Curve curve(edge);
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
			HLine* new_object = new HLine(PS, PE, &wxGetApp().current_color);
			sketch->Add(new_object, NULL);
		}
		break;

		case GeomAbs_Circle:
			// make an arc
		{
			double uStart = curve.FirstParameter();
			double uEnd = curve.LastParameter();
			gp_Pnt PS;
			gp_Vec VS;
			curve.D1(uStart, PS, VS);
			gp_Pnt PE;
			gp_Vec VE;
			curve.D1(uEnd, PE, VE);
			gp_Circ circle = curve.Circle();
			if(curve.IsPeriodic())
			{
				double period = curve.Period();
				double uHalf = uStart + period/2;
				gp_Pnt PH;
				gp_Vec VH;
				curve.D1(uHalf, PH, VH);
				{
					HArc* new_object = new HArc(PS, PH, circle, &wxGetApp().current_color);
					sketch->Add(new_object, NULL);
				}
				{
					HArc* new_object = new HArc(PH, PE, circle, &wxGetApp().current_color);
					sketch->Add(new_object, NULL);
				}
			}
			else
			{
				HArc* new_object = new HArc(PS, PE, circle, &wxGetApp().current_color);
				sketch->Add(new_object, NULL);
			}
		}
		break;

		default:
		{
			// make lots of small lines
			BRepTools::Clean(edge);
			BRepMesh::Mesh(edge, deviation);

			TopLoc_Location L;
			Handle(Poly_Polygon3D) Polyg = BRep_Tool::Polygon3D(edge, L);
			if (!Polyg.IsNull()) {
				const TColgp_Array1OfPnt& Points = Polyg->Nodes();
				Standard_Integer po;
				gp_Pnt prev_p;
				int i = 0;
				for (po = Points.Lower(); po <= Points.Upper(); po++, i++) {
					gp_Pnt p = (Points.Value(po)).Transformed(L);
					if(i != 0)
					{
						HLine* new_object = new HLine(prev_p, p, &wxGetApp().current_color);
						sketch->Add(new_object, NULL);
					}
					prev_p = p;
				}
			}
		}
		break;
	}

	return true;
}

void ConvertSketchesToFace::Run(){
	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			std::list<TopoDS_Shape> faces;
			if(ConvertSketchToFaceOrWire(object, faces, true))
			{
				wxGetApp().CreateUndoPoint();
				for(std::list<TopoDS_Shape>::iterator It2 = faces.begin(); It2 != faces.end(); It2++)
				{
					TopoDS_Shape& face = *It2;
					wxGetApp().Add(new CFace(TopoDS::Face(face)), NULL);
				}
				wxGetApp().Changed();
			}
		}
	}
}

void MakeLineArcsToSketch::Run(){
	std::list<HeeksObj*> objects_to_delete;

	CSketch* sketch = new CSketch();

	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case LineType:
			case ArcType:
			case CircleType:
			case EllipseType:
			case SplineType:
				{
					HeeksObj* new_object = object->MakeACopy();
					objects_to_delete.push_back(object);
					sketch->Add(new_object, NULL);
				}
				break;
			default:
				break;
		}
	}

	wxGetApp().Add(sketch, NULL);
	wxGetApp().Remove(objects_to_delete);
}

void MakeEdgesToSketch::Run(){
	CSketch* sketch = new CSketch();

	std::list<HeeksObj*>::const_iterator It;
	for(It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch(object->GetType()){
			case EdgeType:
				{
					ConvertEdgeToSketch2(((CEdge*)object)->Edge(), sketch, FaceToSketchTool::deviation);
				}
				break;
			default:
				break;
		}
	}

	wxGetApp().Add(sketch, NULL);
}

static CSketch* sketch_for_arcs_to_lines = NULL;
static bool point_found_for_arc_to_lines = false;
static double point_for_arcs_to_lines[3];
static void callback_for_arcs_to_lines(const double* p)
{
	if(point_found_for_arc_to_lines)
	{
		HLine* new_object = new HLine(make_point(p), make_point(point_for_arcs_to_lines), &wxGetApp().current_color);
		sketch_for_arcs_to_lines->Add(new_object, NULL);
	}
	point_found_for_arc_to_lines = true;
	memcpy(point_for_arcs_to_lines, p, 3*sizeof(double));
}

HeeksObj* SplitArcsIntoLittleLines(HeeksObj* sketch)
{
	CSketch* new_sketch = new CSketch;
	sketch_for_arcs_to_lines = new_sketch;
	point_found_for_arc_to_lines = false;
	for(HeeksObj* o = sketch->GetFirstChild(); o; o = sketch->GetNextChild())
	{
		//reset state machine
		point_found_for_arc_to_lines = false;
		o->GetSegments(callback_for_arcs_to_lines, 0.2/FaceToSketchTool::deviation);
	}
	return new_sketch;
}

void SketchesArcsToLines::Run(){
	std::list<HeeksObj*> copy_of_marked_list = wxGetApp().m_marked_list->list();
	std::list<HeeksObj*> objects_to_delete;
	std::list<HeeksObj*> new_objects;
	for(std::list<HeeksObj*>::const_iterator It = copy_of_marked_list.begin(); It != copy_of_marked_list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			HeeksObj* new_object = SplitArcsIntoLittleLines(object);
			new_objects.push_back(new_object);
			objects_to_delete.push_back(object);
		}
	}

	wxGetApp().Remove(objects_to_delete);
}

void CombineSketches::Run(){
	CSketch* sketch1 = NULL;
	std::list<HeeksObj*>::const_iterator It;
	std::list<HeeksObj*> copy_of_marked_list = wxGetApp().m_marked_list->list();

	for(It = copy_of_marked_list.begin(); It != copy_of_marked_list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			if(sketch1)
			{
				std::list<HeeksObj*> new_lines_and_arcs;
				for(HeeksObj* o = object->GetFirstChild(); o; o = object->GetNextChild())
				{
					new_lines_and_arcs.push_back(o->MakeACopy());
				}
				wxGetApp().Remove(object);
				((ObjList*)sketch1)->Add(new_lines_and_arcs);
			}
			else
			{
				sketch1 = (CSketch*)object;
			}
		}
	}

	wxGetApp().Repaint();
}

void UniteSketches::Run(){
#if 1 // use MultiPoly, doesn't work yet
	std::list<HeeksObj*>::const_iterator It;
	std::list<HeeksObj*> copy_of_marked_list = wxGetApp().m_marked_list->list();

	std::list<CSketch*> sketches;

	for(It = copy_of_marked_list.begin(); It != copy_of_marked_list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			sketches.push_back((CSketch*)object);
		}
	}

	std::vector<TopoDS_Face> faces = MultiPoly(sketches);

	for(std::vector<TopoDS_Face>::iterator It = faces.begin(); It != faces.end(); It++)
	{
		TopoDS_Face &face = *It;
		HeeksObj* new_object = CShape::MakeObject(face, _("Test Face, Sketches United"), SOLID_TYPE_UNKNOWN, HeeksColor(64, 51, 51));
		wxGetApp().Add(new_object, NULL);
	}

	wxGetApp().Repaint();
#else // use Polygon
//bool UnionPolygons(std::vector<LineSegment> &lines_vector,
//		std::list<CPolygon> & result_list);
	std::list<HeeksObj*>::const_iterator It;
	std::list<HeeksObj*> copy_of_marked_list = wxGetApp().m_marked_list->list();

	for(It = copy_of_marked_list.begin(); It != copy_of_marked_list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			std::vector<LineSegment> lines;
			lines.resize(object->GetNumChildren());
			unsigned int i = 0;
			for(HeeksObj* sub_object = object->GetFirstChild(); sub_object != NULL; sub_object = object->GetNextChild(), i++)
			{
				LineSegment line;
				double pos[3];
				sub_object->GetStartPoint(pos);
				line.a = make_point(pos);
				sub_object->GetEndPoint(pos);
				line.b = make_point(pos);
				lines[i] = line;
			}

			UnionPolygons(lines, result_list);

		}
	}
#endif
}

void GroupSelected::Run(){
	if(wxGetApp().m_marked_list->list().size() < 2)
	{
		return;
	}

	CGroup* new_group = new CGroup;
	std::list<HeeksObj*> copy_of_marked_list = wxGetApp().m_marked_list->list();

	wxGetApp().Remove(copy_of_marked_list);

	std::list<HeeksObj*>::iterator it;
	for(it = copy_of_marked_list.begin(); it != copy_of_marked_list.end(); it++)
		(*it)->RemoveOwners();

	new_group->Add(copy_of_marked_list);
	wxGetApp().Add(new_group, NULL);
	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().Repaint();
}

void UngroupSelected::Run(){
	if(wxGetApp().m_marked_list->list().size() == 0)return;

	std::list<HeeksObj*> copy_of_marked_list = wxGetApp().m_marked_list->list();
	for(std::list<HeeksObj*>::const_iterator It = copy_of_marked_list.begin(); It != copy_of_marked_list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == GroupType)
		{
			std::list<HeeksObj*> list;
			for(HeeksObj* o = ((CGroup*)object)->GetFirstChild(); o; o = ((CGroup*)object)->GetNextChild())
			{
				list.push_back(o);
			}
			wxGetApp().Remove(list);
			((ObjList*)&wxGetApp())->Add(list);
			wxGetApp().Remove(object);
		}
	}

	wxGetApp().m_marked_list->Clear(true);
	wxGetApp().Repaint();
}
