// Loop.cpp

#include "stdafx.h"
#include "Loop.h"
#include "Face.h"
#include "Edge.h"

CLoop::CLoop(CFace* face, bool orientation, std::list<CEdge*> edges, bool is_outer) {
	m_pface = face;
	//if(orientation)
	//{
		m_edges = edges;
	//}
	//else
	//{
	//	for(std::list<CEdge*>::reverse_iterator RIt = edges.rbegin(); RIt != edges.rend(); RIt++)
	//	{
	//		m_edges.push_back(*RIt);
	//	}
	//}
	m_is_outer = is_outer;
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

