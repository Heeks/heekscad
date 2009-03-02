// Loop.h

// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#ifdef WIN32
#pragma once
#endif

#include "../interface/HeeksObj.h"

class CFace;
class CEdge;

class CLoop : public HeeksObj{
public:
	CFace* m_pface;
	std::list<CEdge*> m_edges;
	std::list<CEdge*>::iterator m_edgeIt;
	bool m_is_outer;

	CLoop(CFace* face, bool orientation, std::list<CEdge*> edges, bool is_outer);
	~CLoop();

	// HeeksObj's virtual functions
	HeeksObj *MakeACopy(void)const{ return new CLoop(*this);}

	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
	CEdge* GetEdge(int index);
};

