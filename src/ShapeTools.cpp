// ShapeTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "ShapeTools.h"
#include "Vertex.h"

void CreateFacesAndEdges(TopoDS_Shape shape, CFaceList* faces, CEdgeList* edges, CVertexList* vertices)
{
	// create the face objects
	TopTools_MapOfShape edgeMap;
	for (TopExp_Explorer expFace(shape, TopAbs_FACE); expFace.More(); expFace.Next())
	{
		TopoDS_Face F = TopoDS::Face(expFace.Current());
		CFace* new_object = new CFace(F);
		faces->Add(new_object, NULL);
		
		// create the edge objects from each face
		for (TopExp_Explorer expEdge(F, TopAbs_EDGE); expEdge.More(); expEdge.Next())
		{
			edgeMap.Add(expEdge.Current());
		}
	}

	// create the vertex object
	for (TopExp_Explorer expVertex(shape, TopAbs_VERTEX); expVertex.More(); expVertex.Next())
	{
		TopoDS_Vertex v = TopoDS::Vertex(expVertex.Current());
		CVertex* new_object = new CVertex(v);
		vertices->Add(new_object, NULL);
	}

	// create the edge objects
	for (TopExp_Explorer expEdge(shape, TopAbs_EDGE); expEdge.More(); expEdge.Next())
	{
		TopoDS_Edge E = TopoDS::Edge(expEdge.Current());
		CEdge* new_object = new CEdge(E);
		edges->Add(new_object, NULL);
	}

	// make an edge map for each face
	std::map< CFace*, TopTools_MapOfShape*> face_edge_maps;
	for(HeeksObj* object = faces->GetFirstChild(); object; object = faces->GetNextChild())
	{
		CFace* face = (CFace*)object;
		TopTools_MapOfShape* newEdgeMap = new TopTools_MapOfShape;
		face_edge_maps.insert( std::make_pair(face, newEdgeMap) );

		for (TopExp_Explorer expEdge(face->Face(), TopAbs_EDGE); expEdge.More(); expEdge.Next())
		{
			const TopoDS_Shape &E = expEdge.Current();
			newEdgeMap->Add(E);
		}
	}

	// for each edge, find which faces it belongs to
	for(HeeksObj* object = edges->GetFirstChild(); object; object = edges->GetNextChild())
	{
		CEdge* edge = (CEdge*)object;

		const TopoDS_Shape &E = edge->Edge();

		// test each face
		for(std::map< CFace*, TopTools_MapOfShape*>::iterator It = face_edge_maps.begin(); It != face_edge_maps.end(); It++)
		{
			CFace* face = It->first;
			TopTools_MapOfShape *map = It->second;
			if(map->Contains(E)){
				face->m_edges.push_back(edge);
			}
		}
	}

	// create the face loops
	std::set<HeeksObj*> edges_to_delete;
	for(HeeksObj* object = faces->GetFirstChild(); object; object = faces->GetNextChild())
	{
		CFace* face = (CFace*)object;
		const TopoDS_Shape &F = face->Face();

		TopoDS_Wire outerWire=BRepTools::OuterWire(TopoDS::Face(F));

		for (TopExp_Explorer expWire(F, TopAbs_WIRE); expWire.More(); expWire.Next())
		{
			const TopoDS_Shape &W = expWire.Current();
			bool is_outer = W.IsSame(outerWire) != 0;
			std::list<CEdge*> edges;

			TopAbs_Orientation wo = W.Orientation();
			bool bwo = (wo == TopAbs_FORWARD);
			TopAbs_Orientation fwo = F.Orientation();
			bool bfwo = (fwo == TopAbs_FORWARD);
			bool ooooo = (bwo == bfwo);

			for(BRepTools_WireExplorer expEdge(TopoDS::Wire(W)); expEdge.More(); expEdge.Next())
			{
				// look through the face's existing edges to find the CEdge*
				for(CEdge* edge = face->GetFirstEdge(); edge; edge = face->GetNextEdge())
				{
					const TopoDS_Shape &E = edge->Edge();
					if(E.IsSame(expEdge.Current())) {
						edges.push_back(edge);
						edge->m_faces.push_back(face);
						break;
					}
				}
			}

			std::list<CEdge*> edges_for_loop;
			for(std::list<CEdge*>::iterator It = edges.begin(); It != edges.end(); It++){
				CEdge* edge = *It;
				if(edge->m_faces.size() == 2 && edge->m_faces.front() == edge->m_faces.back()){
					if(edges_for_loop.size() > 0){
						CLoop* new_loop = new CLoop(face, ooooo, edges_for_loop, is_outer);
						face->m_loops.push_back(new_loop);
						edges_for_loop.clear();
					}
					edges_to_delete.insert(edge);
					face->m_edges.clear();
				}
				else{
					edges_for_loop.push_back(edge);
				}
			}
			if(edges_for_loop.size() > 0){
				CLoop* new_loop = new CLoop(face, ooooo, edges_for_loop, is_outer);
				face->m_loops.push_back(new_loop);
			}
		}
	}

	// delete edges
	edges->Clear(edges_to_delete);

	// calculate face senses
	for(HeeksObj* object = edges->GetFirstChild(); object; object = edges->GetNextChild())
	{
		CEdge* edge = (CEdge*)object;
		const TopoDS_Shape &E1 = edge->Edge();
		for(CFace* face = edge->GetFirstFace(); face; face = edge->GetNextFace())
		{
			bool sense = false;
			const TopoDS_Shape &F = face->Face();
			for (TopExp_Explorer expEdge(F, TopAbs_EDGE); expEdge.More(); expEdge.Next())
			{
				const TopoDS_Shape &E2 = expEdge.Current(); // this is the edge on the face
				if(E1.IsSame(E2))// same edge, but maybe different orientation
				{
					if(E1.IsEqual(E2))sense = true; // same orientation too
					break; // same edge ( ignore the face's other edges )
				}
			}
			edge->m_face_senses.push_back(sense); // one sense for each face, for each edge
		}
	}

	// delete the maps
	for(std::map< CFace*, TopTools_MapOfShape*>::iterator It = face_edge_maps.begin(); It != face_edge_maps.end(); It++)
	{
		TopTools_MapOfShape *map = It->second;
		delete map;
	}

#if _DEBUG
	// test InFaceSense
	for(HeeksObj* object = edges->GetFirstChild(); object; object = edges->GetNextChild())
	{
		CEdge* edge = (CEdge*)object;

		for(CFace* face = edge->GetFirstFace(); face; face = edge->GetNextFace())
		{
			bool in_face_sense = edge->InFaceSense(face);

			bool here = false;
			here = true;
		}
	}
#endif
}

