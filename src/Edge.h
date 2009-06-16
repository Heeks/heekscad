// Edge.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "../interface/HeeksObj.h"
#include <TopoDS_Edge.hxx>

class CFace;

class CEdge:public HeeksObj{
private:
	CBox m_box;
	TopoDS_Edge m_topods_edge;
	static wxIcon* m_icon;
	double m_start_x;
	double m_start_y;
	double m_start_z;
	double m_end_x;
	double m_end_y;
	double m_end_z;
	double m_start_u;
	double m_end_u;
	double m_start_tangent_x;
	double m_start_tangent_y;
	double m_start_tangent_z;
	int m_isClosed;
	int m_isPeriodic;
	bool m_orientation;

public:
	int m_temp_attr; // not saved with the model
	std::list<CFace*>::iterator m_faceIt;
	std::list<CFace*> m_faces;
	std::list<bool> m_face_senses;
	double m_midpoint[3];
	bool m_midpoint_calculated;

	CEdge(const TopoDS_Edge &edge);
	~CEdge();

	int GetType()const{return EdgeType;}
	long GetMarkingMask()const{return MARKING_FILTER_EDGE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	HeeksObj *MakeACopy(void)const{ return new CEdge(*this);}
	wxString GetIcon(){return wxGetApp().GetResFolder() + _T("/icons/edge");}
	const wxChar* GetTypeString(void)const{return _("Edge");}
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void WriteXML(TiXmlNode *root);
	bool UsesID(){return true;}

	const TopoDS_Shape &Edge(){return m_topods_edge;}
	void Blend(double radius);
	CFace* GetFirstFace();
	CFace* GetNextFace();
	int GetCurveType();
	void GetCurveParams(double* start, double* end, double* uStart, double* uEnd, int* Reversed);
	void GetCurveParams2(double *uStart, double *uEnd, int *isClosed, int *isPeriodic);
	bool InFaceSense(CFace* face);
	void Evaluate(double u, double *p, double *tangent);
	bool GetLineParams(double *d6);
	bool GetCircleParams(double *d7);
	bool Orientation();
};

