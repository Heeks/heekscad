// Cylinder.cpp

#include "stdafx.h"
#include "Cylinder.h"
#include <BRepPrimAPI_MakeCylinder.hxx>
#include "PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "Gripper.h"
#include "MarkedList.h"

wxIcon* CCylinder::m_icon = NULL;

CCylinder::CCylinder(const gp_Ax2& pos, double radius, double height, const wxChar* title):m_pos(pos), m_radius(radius), m_height(height), CSolid(BRepPrimAPI_MakeCylinder(pos, radius, height), title)
{
}

wxIcon* CCylinder::GetIcon()
{
	if(m_icon == NULL){wxString exe_folder = wxGetApp().GetExeFolder();	m_icon = new wxIcon(exe_folder + _T("/icons/cyl.png"), wxBITMAP_TYPE_PNG);}
	return m_icon;
}

HeeksObj *CCylinder::MakeACopy(void)const
{
	return new CCylinder(*this);
}

static void on_set_centre(const gp_Pnt &vt, HeeksObj* object){
	gp_Trsf mat;
	mat.SetTranslation ( gp_Vec ( ((CCylinder*)object)->m_pos.Location(), vt ) );
	((CCylinder*)object)->m_pos.Transform(mat);
}

static void on_set_radius(double value, HeeksObj* object){
	((CCylinder*)object)->m_radius = value;
}

static void on_set_height(double value, HeeksObj* object){
	((CCylinder*)object)->m_height = value;
}

void CCylinder::ModifyByMatrix(const double* m, bool for_undo){
	if(!for_undo){
		gp_Trsf mat = make_matrix(m);
		gp_Ax2 new_pos = m_pos.Transformed(mat);
		double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
		double new_radius = fabs(m_radius * scale);
		double new_height = fabs(m_height * scale);
		wxGetApp().AddUndoably(new CCylinder(new_pos, new_radius, new_height, m_title.c_str()), m_owner, NULL);
		wxGetApp().DeleteUndoably(this);
	}
}

void CCylinder::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);

	list->push_back(new PropertyVertex(_T("centre pos"), m_pos.Location(), this, on_set_centre));
	list->push_back(new PropertyDouble(_T("radius"), m_radius, this, on_set_radius));
	list->push_back(new PropertyDouble(_T("height"), m_height, this, on_set_height));
}

void CCylinder::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_radius);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_radius + z_dir.XYZ() * m_height);
	gp_Pnt pmxz(o.XYZ() + m_pos.XDirection().XYZ() * (-m_radius) + z_dir.XYZ() * m_height);
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);
	list->push_back(GripperTypeTranslate);
	list->push_back(o.X());
	list->push_back(o.Y());
	list->push_back(o.Z());
	list->push_back(GripperTypeObjectScaleXY);
	list->push_back(px.X());
	list->push_back(px.Y());
	list->push_back(px.Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(pyz.X());
	list->push_back(pyz.Y());
	list->push_back(pyz.Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(pmxz.X());
	list->push_back(pmxz.Y());
	list->push_back(pmxz.Z());
	list->push_back(GripperTypeObjectScaleZ);
	list->push_back(pz.X());
	list->push_back(pz.Y());
	list->push_back(pz.Z());
}

void CCylinder::OnApplyProperties()
{
	CCylinder* new_object = new CCylinder(m_pos, m_radius, m_height, m_title.c_str());
	wxGetApp().StartHistory(_T("Edit Cylinder"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(this);
	wxGetApp().EndHistory();
	wxGetApp().m_marked_list->Clear();
	wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();
}


bool CCylinder::GetScaleAboutMatrix(double *m)
{
	gp_Trsf mat = make_matrix(m_pos.Location(), m_pos.XDirection(), m_pos.YDirection());
	extract(mat, m);
	return true;
}

void CCylinder::Stretch(const double *p, const double* shift, double* new_position)
{
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_radius);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);

	bool make_a_new_cylinder = false;

	if(px.IsEqual(vp, wxGetApp().m_geom_tol)){
		px = px.XYZ() + vshift.XYZ();
		extract(px, new_position);
		double new_x = gp_Vec(px.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		double new_y = gp_Vec(px.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		make_a_new_cylinder = true;
		m_radius = sqrt(new_x * new_x + new_y * new_y);
	}
	else if(pz.IsEqual(vp, wxGetApp().m_geom_tol)){
		pz = pz.XYZ() + vshift.XYZ();
		extract(pz, new_position);
		double new_height = gp_Vec(pz.XYZ()) * gp_Vec(z_dir) - gp_Vec(o.XYZ()) * gp_Vec(z_dir);
		if(new_height > 0){
			make_a_new_cylinder = true;
			m_height = new_height;
		}
	}

	if(make_a_new_cylinder)
	{
		CCylinder* new_object = new CCylinder(m_pos, m_radius, m_height, m_title.c_str());
		wxGetApp().StartHistory(_T("Stretch Cylinder"));
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().DeleteUndoably(this);
		wxGetApp().EndHistory();
		wxGetApp().m_marked_list->Clear();
		wxGetApp().m_marked_list->Add(new_object);
	}
}
