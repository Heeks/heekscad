// Shape.h

#pragma once

#include "../interface/Material.h"
#include <TopoDS_Solid.hxx>
#include "ShapeData.h"

class CFace;
class CEdge;

class CFaceList: public ObjList{
	static wxIcon* m_icon;
public:
	const wxChar* GetTypeString(void)const{return _("Faces");}
	HeeksObj *MakeACopy(void)const{ return new CFaceList(*this);}
	wxIcon* GetIcon();
};

class CEdgeList: public ObjList{
	static wxIcon* m_icon;
public:
	const wxChar* GetTypeString(void)const{return _("Edges");}
	HeeksObj *MakeACopy(void)const{ return new CEdgeList(*this);}
	wxIcon* GetIcon();
};

class CShape:public ObjList{
protected:
	int m_gl_list;
	CBox m_box;
	TopoDS_Shape m_shape;
	Material m_material;
	static wxIcon* m_icon;

	void create_faces_and_edges();
	void delete_faces_and_edges();

public:
	static bool m_solids_found; // a flag for xml writing
	CFaceList* m_faces;
	CEdgeList* m_edges;
	wxString m_title;
	HeeksColor m_color;

	CShape(const TopoDS_Shape &shape, const wxChar* title, const HeeksColor& col);
	CShape(const CShape& s);
	~CShape();

	virtual const CShape& operator=(const CShape& s);

	int GetType()const{return SolidType;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	void KillGLLists(void);
	bool ModifyByMatrix(const double* m);
	const wxChar* GetShortString(void)const{return m_title.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	double Area()const;
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void CopyFrom(const HeeksObj* object);
	void WriteXML(TiXmlElement *root);

	const TopoDS_Shape &Shape(){return m_shape;}

	CFace* find(const TopoDS_Face &face);

	static void CutShapes(const std::list<HeeksObj*> &list);
	static void FuseShapes(const std::list<HeeksObj*> &list);
	static void CommonShapes(const std::list<HeeksObj*> &list);
	static bool ImportSolidsFile(const wxChar* filepath, bool undoably, std::map<int, CShapeData> *index_map = NULL);
	static bool ExportSolidsFile(const std::list<HeeksObj*>& objects, const wxChar* filepath, std::map<int, CShapeData> *index_map = NULL);
	static HeeksObj* MakeObject(const TopoDS_Shape &shape, const wxChar* title, SolidTypeEnum solid_type, const HeeksColor& col);
	static bool IsTypeAShape(int t);

	virtual void SetXMLElement(TiXmlElement* element){}
	virtual void SetFromXMLElement(TiXmlElement* pElem){}
};