// Triangles.h

#pragma once

#include "../interface/ObjList.h"
#include "../interface/Tool.h"
#include "../interface/HeeksColor.h"

class CTri;

class CTriangles : public ObjList
{
	private:
	CBox m_box;
	bool m_gl_list_exists;
	int m_gl_list;
	double m_left;
	double m_front;
	double m_xs;
	double m_ys;
	double m_top;
	HeeksColor color;

	void delete_all_triangles();

	public:
	static int cull_mode; // 0 - none, GL_BACK - cull back faces, GL_FRONT - cull front faces

	CTriangles(const HeeksColor &col);
	CTriangles(const CTriangles& t) {operator=(t);}
	virtual ~CTriangles();
	const CTriangles& operator=(const CTriangles& rhs);

	// HeeksObj's virtual functions
	void glCommands(bool select, bool marked, bool no_color);
	int GetType()const{return TrianglesType;}
	void GetBox(CBox &box);
	const char* GetTypeString(void)const{return "Triangles";}
	void SetColor(const HeeksColor &col);
	const HeeksColor* GetColor()const{return &color;}
	bool CanAdd(HeeksObj* object);
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void ModifyByMatrix(const double *m);
	HeeksObj *MakeACopy(void)const;
	void GetGripperPositions(std::list<double> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	wxIcon* GetIcon();
	void MakeTriangles(const double *mat, void(*call_back)(const double* vt0, const double* vt1, const double* vt2, const double* n0, const double* n1, const double* n2));
	const char* GetTexture();
	bool GetStartPoint(double* pos);
	void KillGLLists(void);

	// member functions
	unsigned int size(){return m_objects.size();}
};

class SetCullFaceTool : public Tool
{
	int m_mode;

	public:
	SetCullFaceTool(int mode): m_mode(mode) {}
	const char* GetTitle();
	void Run() {CTriangles::cull_mode = m_mode;}
};

