// Loop.h

// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"

class CFace;
class CEdge;

class CLoop : public HeeksObj{
public:
	TopoDS_Wire m_topods_wire;
	CFace* m_pface;
	std::list<CEdge*> m_edges;
	std::list<CEdge*>::iterator m_edgeIt;
	bool m_is_outer;

	CLoop(const TopoDS_Wire &wire);
	~CLoop();

	// HeeksObj's virtual functions
	HeeksObj *MakeACopy(void)const{ return new CLoop(*this);}

	const TopoDS_Wire &Wire(){return m_topods_wire;}
	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
	CEdge* GetEdge(int index);
};

