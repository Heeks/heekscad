// Vertex.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"

class CFace;
class CEdge;

class HVertex:public HeeksObj{
private:
	TopoDS_Vertex m_topods_vertex;
	std::list<CEdge*>::iterator m_edgeIt;

	void FindEdges();

public:
	std::list<CEdge*> m_edges;
	std::list<CFace*> m_faces;
	double m_point[3];

	HVertex(const TopoDS_Vertex &vertex);
	~HVertex();

	int GetType()const{return VertexType;}
	long GetMarkingMask()const{return MARKING_FILTER_VERTEX;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	HeeksObj *MakeACopy(void)const{ return new HVertex(*this);}
	const wxBitmap &GetIcon();
	const wxChar* GetTypeString(void)const{return _("Vertex");}
	bool UsesID(){return true;}

	const TopoDS_Shape &Vertex(){return m_topods_vertex;}
	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
	CShape* GetParentBody();
};

