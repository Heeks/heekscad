// DynamicSolid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Shape.h"
#include "DynamicSolid.h"
#include "FaceTools.h"


DynamicSolid::DynamicSolid()
{
	m_faces = new CFaceList;
	m_faces->SetSkipForUndo(true);
	m_edges = new CEdgeList;
	m_edges->SetSkipForUndo(true);
	m_vertices = new CVertexList;
	m_vertices->SetSkipForUndo(true);
	Add(m_faces, NULL);
	Add(m_edges, NULL);
	Add(m_vertices, NULL);
}

DynamicSolid::~DynamicSolid()
{

}

void DynamicSolid::SetShapes(std::list<TopoDS_Shape> shapes)
{
	m_shapes = shapes;

	m_faces->Clear();
	m_edges->Clear();
	m_vertices->Clear();

	std::list<TopoDS_Shape>::iterator it;
	for(it = shapes.begin(); it != shapes.end(); it++)
	{
		CreateFacesAndEdges(*it, m_faces, m_edges, m_vertices);
	}
}

void DynamicSolid::DrawShapes()
{
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);

	double pixels_per_mm = wxGetApp().GetPixelScale();
	std::list<TopoDS_Shape>::iterator it;

	for(it = m_shapes.begin(); it != m_shapes.end(); it++)
	{
		for (TopExp_Explorer expFace(*it, TopAbs_FACE); expFace.More(); expFace.Next())
		{
			TopoDS_Face F = TopoDS::Face(expFace.Current());
			MeshFace(F,pixels_per_mm);
			DrawFaceWithCommands(F);
		}
	}

	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);
	
}

void DynamicSolid::ReloadPointers()
{
	HeeksObj* child = GetFirstChild();
	while(child)
	{
		CFaceList* faces = dynamic_cast<CFaceList*>(child);
		if(faces)
			m_faces = faces;
		CEdgeList* edges = dynamic_cast<CEdgeList*>(child);
		if(edges)
			m_edges = edges;
		CVertexList* vertices = dynamic_cast<CVertexList*>(child);
		if(vertices)
			m_vertices = vertices;
		child = GetNextChild();
	}
}

