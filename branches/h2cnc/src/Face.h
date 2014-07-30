// Face.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class CEdge;
class CLoop;
class CShape;
class CNurbSurfaceParams;

class CFace:public HeeksObj{
private:
	TopoDS_Face m_topods_face;
#if _DEBUG
	double m_pos_x;
	double m_pos_y;
	double m_pos_z;
	double m_normal_x;
	double m_normal_y;
	double m_normal_z;
	bool m_orientation;
#endif
	int m_marking_gl_list; // simply has material commands, inserted in the parent body's display list

public:
	CBox m_box;
	int m_temp_attr; // not saved with the model
	std::list<CEdge*>::iterator m_edgeIt;
	std::list<CEdge*> m_edges;
	std::list<CLoop*>::iterator m_loopIt;
	std::list<CLoop*> m_loops;

	CFace();
	CFace(const TopoDS_Face &face);
	~CFace();

	int GetType()const{return FaceType;}
	long GetMarkingMask()const{return MARKING_FILTER_FACE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wxBitmap &GetIcon();
	HeeksObj *MakeACopy(void)const{ return new CFace(*this);}
	const wxChar* GetTypeString(void)const{return _("Face");}
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = false);
	double Area()const;
	void ModifyByMatrix(const double* m);
	void WriteXML(TiXmlNode *root);
	void GetProperties(std::list<Property *> *list);
	bool UsesID(){return true;}
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void GetGripperPositionsTransformed(std::list<GripData> *list, bool just_for_endof);

	const TopoDS_Face &Face(){return m_topods_face;}
	gp_Dir GetMiddleNormal(gp_Pnt *pos = NULL)const;
	gp_Dir GetNormalAtUV(double u, double v, gp_Pnt *pos = NULL)const;
	bool GetUVAtPoint(const gp_Pnt &pos, double *u, double *v)const;
	bool GetClosestPoint(const gp_Pnt &pos, gp_Pnt &closest_pnt)const;
	bool GetClosestSurfacePoint(const gp_Pnt &pos, gp_Pnt &closest_pnt)const;
	void GetPlaneParams(gp_Pln &p);
	void GetCylinderParams(gp_Cylinder &c);
	void GetSphereParams(gp_Sphere &s);
	void GetConeParams(gp_Cone &c);
	void GetTorusParams(gp_Torus &t);
	bool GetNurbSurfaceParams(CNurbSurfaceParams* params);
	int GetSurfaceType();
	bool IsAPlane(gp_Pln *returned_plane);
	wxString GetSurfaceTypeStr();
	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
	CLoop* GetFirstLoop();
	CLoop* GetNextLoop();
	bool Orientation();
	void GetUVBox(double *uv_box);
	void GetSurfaceUVPeriod(double *uv, bool *isUPeriodic, bool *isVPeriodic);
	CShape* GetParentBody();
	void MakeSureMarkingGLListExists();
	void KillMarkingGLList();
	void UpdateMarkingGLList(bool marked);
};

class FaceToSketchTool:public Tool
{
public:
	const wxChar* GetTitle(){return _("Make a sketch from face");}
	wxString BitmapPath(){return _T("face2sketch");}
	void Run();

	static double deviation;
};


