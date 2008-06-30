// Tri.cpp: implementation of the CTri class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tri.h"
#include "../interface/PropertyString.h"
#include "Triangles.h"

CTri::CTri(void):m_norm_exists(false)
{
	m_p[0] = gp_Pnt(0,0,0);
	m_p[1] = gp_Pnt(0,0,0);
	m_p[2] = gp_Pnt(0,0,0);
}

CTri::CTri(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2):m_norm_exists(false)
{
}

CTri::CTri(const CTri& tri){
	operator=(tri);
}

CTri::~CTri()
{
}

const CTri& CTri::operator=(const CTri& tri){
	m_p[0] = tri.m_p[0];
	m_p[1] = tri.m_p[1];
	m_p[2] = tri.m_p[2];

	m_box = tri.m_box;
	m_norm_exists = tri.m_norm_exists;
	if(m_norm_exists){
		m_norm = tri.m_norm;
	}

	return *this;
}

HeeksObj *CTri::MakeACopy(void)const{
	return new CTri(*this);
}

static wxIcon* icon = NULL;

wxIcon* CTri::GetIcon()
{
	if(icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		icon = new wxIcon(exe_folder + "/icons/tri.png", wxBITMAP_TYPE_PNG);
	}
	return icon;
}

void CTri::glCommands(bool select, bool marked, bool no_color)
{
	if(!no_color)
	{	
		glEnable(GL_LIGHTING);
		HeeksColor(0, 99, 0).glMaterial(1, GL_FRONT);
		HeeksColor(230, 0, 184).glMaterial(1, GL_BACK);
		if(CTriangles::cull_mode){
			glEnable(GL_CULL_FACE);
			glCullFace(CTriangles::cull_mode);
		}
	}

	if(!m_norm_exists){
		gp_Vec v1(m_p[0], m_p[1]);
		gp_Vec v2(m_p[1], m_p[2]);
        m_norm = (v1^v2).Normalized();
		m_norm_exists = true;
	}

	glBegin(GL_TRIANGLES);
	glNormal3d(m_norm.X(), m_norm.Y(), m_norm.Z());
	glVertex3d(m_p[0].X(), m_p[0].Y(), m_p[0].Z());
	glVertex3d(m_p[1].X(), m_p[1].Y(), m_p[1].Z());
	glVertex3d(m_p[2].X(), m_p[2].Y(), m_p[2].Z());
	glEnd();

	if(!no_color){
		glDisable(GL_LIGHTING);
		if(CTriangles::cull_mode){
			glDisable(GL_CULL_FACE);
		}
		glDisable(GL_BLEND);
	}
}

void CTri::GetBox(CBox &box){
	if(!m_box.m_valid){
		for(int i = 0; i<3; i++)m_box.Insert(m_p[i].X(), m_p[i].Y(), m_p[i].Z());
	}

	box.Insert(m_box);
}

void CTri::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	list->push_back(0);
	list->push_back(m_p[0].X());
	list->push_back(m_p[0].Y());
	list->push_back(m_p[0].Z());
	list->push_back(0);
	list->push_back(m_p[1].X());
	list->push_back(m_p[1].Y());
	list->push_back(m_p[1].Z());
	list->push_back(0);
	list->push_back(m_p[2].X());
	list->push_back(m_p[2].Y());
	list->push_back(m_p[2].Z());
}

void CTri::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	m_p[0].Transform(mat);
	m_p[1].Transform(mat);
	m_p[2].Transform(mat);
}

void CTri::GetTools(std::list<Tool*>* f_list, const wxPoint* p)
{
	if(CTriangles::cull_mode != 0)f_list->push_back(new SetCullFaceTool(0));
	if(CTriangles::cull_mode != GL_FRONT)f_list->push_back(new SetCullFaceTool(GL_FRONT));
	if(CTriangles::cull_mode != GL_BACK)f_list->push_back(new SetCullFaceTool(GL_BACK));
}

void CTri::Stretch(const double *p, const double* shift, double* new_position){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	for(int i = 0; i<3; i++)
	{
		if(m_p[i].IsEqual(vp, 0.001)){
			m_p[i] = m_p[i].XYZ() + vshift.XYZ();
			extract(m_p[i], new_position);
			break;
		}
	}
}

bool CTri::GetStartPoint(double* pos)
{
	extract(m_p[0], pos);
	return true;
}

