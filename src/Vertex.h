// Vertex.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "../interface/HeeksObj.h"
#include <TopoDS_Vertex.hxx>

class CFace;
class CEdge;

class CVertex:public HeeksObj{
private:
	TopoDS_Vertex m_topods_vertex;

public:
	std::list<CFace*> m_faces;
	std::list<CEdge*> m_edges;
	std::list<CEdge*>::iterator m_edgeIt;
	double m_point[3];

	CVertex(const TopoDS_Vertex &vertex);
	~CVertex();

	int GetType()const{return VertexType;}
	long GetMarkingMask()const{return MARKING_FILTER_VERTEX;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	HeeksObj *MakeACopy(void)const{ return new CVertex(*this);}
#ifdef WIN32
	wxString GetIcon(){return wxGetApp().GetExeFolder() + _T("/icons/vertex");}
#else
	wxString GetIcon(){return wxGetApp().GetExeFolder() + _T("/../share/heekscad/icons/vertex");}
#endif
	const wxChar* GetTypeString(void)const{return _("Vertex");}
	bool UsesID(){return true;}

	const TopoDS_Shape &Vertex(){return m_topods_vertex;}
	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
};

