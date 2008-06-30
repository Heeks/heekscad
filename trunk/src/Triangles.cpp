// Triangles.cpp

#include "stdafx.h"
#include "Triangles.h"
#include "../interface/Tool.h"
#include "Tri.h"
#include "../interface/PropertyString.h"
#include "bitmaps/triangles.xpm"

#include <fstream>

#define BRIGHT_PINK_INTERIOR

int CTriangles::cull_mode = 0;

CTriangles::CTriangles(const HeeksColor &col): ObjList(), m_gl_list_exists(false)
{
	color = col;
}

CTriangles::~CTriangles()
{
	Clear();
	KillGLLists();
}

static wxIcon* icon = NULL;

wxIcon* CTriangles::GetIcon(){
	if(icon == NULL)icon = new wxIcon(triangles_xpm);
	return icon;
}

void CTriangles::SetColor(const HeeksColor &col)
{
	color = col;
	wxGetApp().WasModified(this);
}

const CTriangles& CTriangles::operator=(const CTriangles& rhs)
{
	m_gl_list_exists = false;
	m_gl_list = 0;
	m_box = rhs.m_box;
	ObjList::operator =(rhs);
	return *this;
}

HeeksObj* CTriangles::MakeACopy(void)const{return new CTriangles(*this);}

void CTriangles::KillGLLists()
{
	if (m_gl_list_exists)
	{
		glDeleteLists(m_gl_list, 1);
		m_gl_list_exists = false;
	}
}

void CTriangles::glCommands(bool select, bool marked, bool no_color)
{
	if(!no_color)
	{	
#ifdef BRIGHT_PINK_INTERIOR
		color.glMaterial(1, GL_FRONT);
		HeeksColor(255, 0, 192).glMaterial(1, GL_BACK);
#else
		color.glMaterial(1);
#endif
		glEnable(GL_LIGHTING);
	}

	if(cull_mode){
		glEnable(GL_CULL_FACE);
		glCullFace(cull_mode);
	}
	if(m_gl_list_exists)
	{
		glCallList(m_gl_list);
	}
	else{
		m_gl_list = glGenLists(1);
		m_gl_list_exists = true;
		glNewList(m_gl_list, GL_COMPILE_AND_EXECUTE);
		ObjList::glCommands(false, false, true);
		glEndList();
	}
	if(cull_mode){
		glDisable(GL_CULL_FACE);
	}

	if(!no_color){
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
	}
}

bool CTriangles::CanAdd(HeeksObj* object)
{
	return (object && object->GetType() == TriType);
}

bool CTriangles::Add(HeeksObj* object, HeeksObj* prev_object)
{
	if (!ObjList::Add(object, prev_object)) return false;
	KillGLLists();
	return true;
}

void CTriangles::GetBox(CBox &box)
{
	if(!m_box.m_valid){
		ObjList::GetBox(m_box);
	}
	box.Insert(m_box);
}

void CTriangles::GetTools(std::list<Tool*>* f_list, const wxPoint* p)
{
	f_list->push_back(NULL);
	if(cull_mode != 0)f_list->push_back(new SetCullFaceTool(0));
	if(cull_mode != GL_FRONT)f_list->push_back(new SetCullFaceTool(GL_FRONT));
	if(cull_mode != GL_BACK)f_list->push_back(new SetCullFaceTool(GL_BACK));
}

void CTriangles::ModifyByMatrix(const double *m)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++) ((CTri*)(*It))->ModifyByMatrix(m);
	gp_Trsf mat = make_matrix(m);
	extract(make_point(&(m_box.m_x[0])).Transformed(mat), &(m_box.m_x[0]));
	extract(make_point(&(m_box.m_x[3])).Transformed(mat), &(m_box.m_x[3]));
	KillGLLists();
}

void CTriangles::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	if (m_objects.size() == 0)return;
	CBox box;
	GetBox(box);
	if(box.m_valid){
		for(int i = 0; i<8; i++){
			double p[3];
			box.vert(i, p);
			list->push_back(0);
			list->push_back(p[0]);
			list->push_back(p[1]);
			list->push_back(p[2]);
		}
	}
}

void CTriangles::GetProperties(std::list<Property *> *list){
	__super::GetProperties(list);
}

bool CTriangles::GetStartPoint(double* pos)
{
	CTri* first = (CTri*)GetFirstChild();
	if (first) return first->GetStartPoint(pos);
	else return false;
}

const char* SetCullFaceTool::GetTitle() {
	switch(m_mode){
case 0:
	return "set no face_culling";
case GL_FRONT:
	return "set front face culling";
case GL_BACK:
	return "set back face culling";
default:
	return "invalid face culling mode!";
		}
	}
