// Loop.h

#pragma once

#include "../interface/HeeksObj.h"

class CFace;
class CEdge;

class CLoop : public HeeksObj{
public:
	CFace* m_pface;
	std::list<CEdge*> m_edges;
	std::list<CEdge*>::iterator m_edgeIt;

	CLoop(CFace* face, std::list<CEdge*> edges);

	// HeeksObj's virtual functions
	HeeksObj *MakeACopy(void)const{ return new CLoop(*this);}

	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
};

