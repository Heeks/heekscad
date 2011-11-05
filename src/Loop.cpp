// Loop.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Loop.h"
#include "Face.h"
#include "Edge.h"

CLoop::CLoop(const TopoDS_Wire &wire) {
	m_topods_wire = wire;
	m_pface = NULL;
	m_is_outer = false;
}

CLoop::~CLoop()
{
}

CEdge* CLoop::GetFirstEdge()
{
	if (m_edges.size()==0) return NULL;
	m_edgeIt = m_edges.begin();
	return *m_edgeIt;
}

CEdge* CLoop::GetNextEdge()
{
	if (m_edges.size()==0 || m_edgeIt==m_edges.end()) return NULL;
	m_edgeIt++;
	if (m_edgeIt==m_edges.end()) return NULL;
	return *m_edgeIt;
}

CEdge* CLoop::GetEdge(int index)
{
	int i = 0;
	for(std::list<CEdge*>::iterator It = m_edges.begin(); It != m_edges.end(); It++, i++)
	{
		if(index == i)return *It;
	}

	return NULL;
}
