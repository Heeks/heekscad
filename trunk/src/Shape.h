// Shape.h

#pragma once

#include "../interface/HeeksObj.h"
#include "../interface/Material.h"
#include <TopoDS_Shape.hxx>

class CFace;
class CEdge;

class CShape:public HeeksObj{
protected:
	int m_gl_list;
	CBox m_box;
	TopoDS_Shape m_shape;
	Material m_material;
	std::string m_title;
	std::list<CFace*> m_faces;
	std::list<CEdge*> m_edges;
	static wxIcon* m_icon;
	bool m_use_one_gl_list;

	void create_face_objects();
	void create_edge_objects();
	void delete_face_objects();
	void delete_edge_objects();

public:
	CShape(const TopoDS_Shape &shape, const char* title, bool use_one_gl_list = false);
	CShape(const CShape& s);
	~CShape();

	virtual const CShape& operator=(const CShape& s);

	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	void KillGLLists(void);
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void ModifyByMatrix(const double* m);
	const char* GetShortString(void)const{return m_title.c_str();}
	const char* GetTypeString(void)const{return "Shape";}
	bool CanEditString(void)const{return true;}
	void OnEditString(const char* str);
	void GetTriangles(void(*callbackfunc)(double* x, double* n), double cusp);
	void GetCentreNormals(void(*callbackfunc)(double area, double *x, double *n));
	void GetTools(std::list<Tool*>* f_list, const wxPoint* p);
	void CopyFrom(const HeeksObj* object);

	const TopoDS_Shape &Shape(){return m_shape;}

	CFace* find(const TopoDS_Face &face);

	static void AddASphere();
	static void AddACube();
	static void AddACylinder();
	static void CutShapes(const std::list<HeeksObj*> &list);
	static void FuseShapes(const std::list<HeeksObj*> &list);
	static void CommonShapes(const std::list<HeeksObj*> &list);
	static bool ImportSolidsFile(const char* filepath);
	static bool ExportSolidsFile(const char* filepath);
	static HeeksObj* MakeObject(const TopoDS_Shape &shape, const char* title, bool use_one_gl_list = false, bool stl_body = false);
	static bool IsTypeAShape(int t);
};