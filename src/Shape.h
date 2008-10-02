// Shape.h

#pragma once

#include "../interface/Material.h"
#include <TopoDS_Shape.hxx>

class CFace;
class CEdge;

class CFaceList: public ObjList{
};

class CEdgeList: public ObjList{
};

class CShapeIndex{
public:
	int id;
	std::map<int, int> edge_index; // map of index to id
	std::map<int, int> face_index; // map of index to id
};

class CShape:public ObjList{
protected:
	int m_gl_list;
	CBox m_box;
	TopoDS_Shape m_shape;
	Material m_material;
	wxString m_title;
	static wxIcon* m_icon;
	bool m_use_one_gl_list;

	void create_face_objects();
	void create_edge_objects();
	void delete_face_objects();
	void delete_edge_objects();

public:
	static bool m_solids_found; // a flag for xml writing
	CFaceList* m_faces;
	CEdgeList* m_edges;

	CShape(const TopoDS_Shape &shape, const wxChar* title, bool use_one_gl_list = false);
	CShape(const CShape& s);
	~CShape();

	virtual const CShape& operator=(const CShape& s);

	void glCommands(bool select, bool marked, bool no_color);
	void KillGLLists(void);
	void ModifyByMatrix(const double* m);
	const wxChar* GetShortString(void)const{return m_title.c_str();}
	const wxChar* GetTypeString(void)const{return _T("Shape");}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	double Area()const;
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void CopyFrom(const HeeksObj* object);
	void WriteXML(TiXmlElement *root);

	const TopoDS_Shape &Shape(){return m_shape;}

	CFace* find(const TopoDS_Face &face);

	static void AddASphere();
	static void AddACube();
	static void AddACylinder();
	static void CutShapes(const std::list<HeeksObj*> &list);
	static void FuseShapes(const std::list<HeeksObj*> &list);
	static void CommonShapes(const std::list<HeeksObj*> &list);
	static bool ImportSolidsFile(const wxChar* filepath, bool undoably, std::map<int, int> *index_map = NULL);
	static bool ExportSolidsFile(const wxChar* filepath, std::map<int, int> *index_map = NULL);
	static HeeksObj* MakeObject(const TopoDS_Shape &shape, const wxChar* title, bool use_one_gl_list = false, bool stl_body = false);
	static bool IsTypeAShape(int t);
};