// Vertex.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Vertex.h"
#include "Face.h"
#include "Solid.h"
#include "Gripper.h"

CVertex::CVertex(const TopoDS_Vertex &vertex):m_topods_vertex(vertex){
	gp_Pnt pos = BRep_Tool::Pnt(vertex);
	extract(pos, m_point);
}

CVertex::~CVertex(){
}

void CVertex::FindEdges()
{
	CShape* body = GetParentBody();
	if(body)
	{
		for(HeeksObj* object = body->m_edges->GetFirstChild(); object; object = body->m_edges->GetNextChild())
		{
			CEdge* e = (CEdge*)object;
			CVertex* v0 = e->GetVertex0();
			CVertex* v1 = e->GetVertex1();
			if(v0 == this || v1 == this)m_edges.push_back(e);
		}
	}
}

const wxBitmap &CVertex::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/vertex.png")));
	return *icon;
}

void CVertex::glCommands(bool select, bool marked, bool no_color){
	// don't render anything, but put a point for selection
	glRasterPos3dv(m_point);
}

void CVertex::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	list->push_back(GripData(GripperTypeTranslate,m_point[0],m_point[1],m_point[2],NULL));
}

CEdge* CVertex::GetFirstEdge()
{
	if (m_edges.size()==0)FindEdges();
	if (m_edges.size()==0) return NULL;
	m_edgeIt = m_edges.begin();
	return *m_edgeIt;
}

CEdge* CVertex::GetNextEdge()
{
	if (m_edges.size()==0 || m_edgeIt==m_edges.end()) return NULL;
	m_edgeIt++;
	if (m_edgeIt==m_edges.end()) return NULL;
	return *m_edgeIt;
}

CShape* CVertex::GetParentBody()
{
	if(Owner() == NULL)return NULL;
	if(Owner()->Owner() == NULL)return NULL;
	if(Owner()->Owner()->GetType() != SolidType)return NULL;
	return (CShape*)(Owner()->Owner());
}
