// Cuboid.cpp

#include "stdafx.h"
#include "Cuboid.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include "PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "Gripper.h"
#include "MarkedList.h"

wxIcon* CCuboid::m_icon = NULL;

CCuboid::CCuboid(const gp_Ax2& pos, double x, double y, double z, const wxChar* title):m_pos(pos), m_x(x), m_y(y), m_z(z), CSolid(BRepPrimAPI_MakeBox(pos, x, y, z), title)
{
}

wxIcon* CCuboid::GetIcon()
{
	if(m_icon == NULL){wxString exe_folder = wxGetApp().GetExeFolder();	m_icon = new wxIcon(exe_folder + _T("/icons/cube.png"), wxBITMAP_TYPE_PNG);}
	return m_icon;
}

HeeksObj *CCuboid::MakeACopy(void)const
{
	return new CCuboid(*this);
}

static void on_set_centre(const gp_Pnt &vt, HeeksObj* object){
	gp_Trsf mat;
	mat.SetTranslation ( gp_Vec ( ((CCuboid*)object)->m_pos.Location(), vt ) );
	((CCuboid*)object)->m_pos.Transform(mat);
}

static void on_set_x(double value, HeeksObj* object){
	((CCuboid*)object)->m_x = value;
}

static void on_set_y(double value, HeeksObj* object){
	((CCuboid*)object)->m_y = value;
}

static void on_set_z(double value, HeeksObj* object){
	((CCuboid*)object)->m_z = value;
}

void CCuboid::ModifyByMatrix(const double* m, bool for_undo){
	if(!for_undo){
		gp_Trsf mat = make_matrix(m);
		gp_Ax2 new_pos = m_pos.Transformed(mat);
		double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
		double new_x = fabs(m_x * scale);
		double new_y = fabs(m_y * scale);
		double new_z = fabs(m_z * scale);
		wxGetApp().AddUndoably(new CCuboid(new_pos, new_x, new_y, new_z, m_title.c_str()), m_owner, NULL);
		wxGetApp().DeleteUndoably(this);
	}
}

void CCuboid::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);

	list->push_back(new PropertyVertex(_T("datum corner"), m_pos.Location(), this, on_set_centre));
	list->push_back(new PropertyDouble(_T("width ( x )"), m_x, this, on_set_x));
	list->push_back(new PropertyDouble(_T("height( y )"), m_y, this, on_set_y));
	list->push_back(new PropertyDouble(_T("depth ( z )"), m_z, this, on_set_z));
}

void CCuboid::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_x);
	gp_Pnt py(o.XYZ() + m_pos.YDirection().XYZ() * m_y);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_z);
	gp_Pnt m1(o.XYZ() + m_pos.XDirection().XYZ() * m_x/2);
	gp_Pnt m2(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y/2);
	gp_Pnt m3(o.XYZ() + m_pos.XDirection().XYZ() * m_x/2 + m_pos.YDirection().XYZ() * m_y);
	gp_Pnt m4(o.XYZ() + m_pos.YDirection().XYZ() * m_y/2);
	gp_Pnt m5(o.XYZ() + m_pos.XDirection().XYZ() * m_x/2 + z_dir.XYZ() * m_z);
	gp_Pnt m6(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y/2 + z_dir.XYZ() * m_z);
	gp_Pnt m7(o.XYZ() + m_pos.XDirection().XYZ() * m_x/2 + m_pos.YDirection().XYZ() * m_y + z_dir.XYZ() * m_z);
	gp_Pnt m8(o.XYZ() + m_pos.YDirection().XYZ() * m_y/2 + z_dir.XYZ() * m_z);
	gp_Pnt m9(o.XYZ() + m_pos.XDirection().XYZ() * m_x + z_dir.XYZ() * m_z/2);
	gp_Pnt ma(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y + z_dir.XYZ() * m_z/2);
	gp_Pnt mb(o.XYZ() + m_pos.YDirection().XYZ() * m_y + z_dir.XYZ() * m_z/2);
	gp_Pnt mc(o.XYZ() + z_dir.XYZ() * m_z/2);
	gp_Pnt pxy(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y);
	gp_Pnt pxz(o.XYZ() + m_pos.XDirection().XYZ() * m_x + z_dir.XYZ() * m_z);
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_y + z_dir.XYZ() * m_z);
	list->push_back(GripperTypeTranslate);
	list->push_back(o.X());
	list->push_back(o.Y());
	list->push_back(o.Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(px.X());
	list->push_back(px.Y());
	list->push_back(px.Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(py.X());
	list->push_back(py.Y());
	list->push_back(py.Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(pz.X());
	list->push_back(pz.Y());
	list->push_back(pz.Z());
	list->push_back(GripperTypeRotate);
	list->push_back(pxy.X());
	list->push_back(pxy.Y());
	list->push_back(pxy.Z());
	list->push_back(GripperTypeRotate);
	list->push_back(pxz.X());
	list->push_back(pxz.Y());
	list->push_back(pxz.Z());
	list->push_back(GripperTypeRotate);
	list->push_back(pyz.X());
	list->push_back(pyz.Y());
	list->push_back(pyz.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m1.X());
	list->push_back(m1.Y());
	list->push_back(m1.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m2.X());
	list->push_back(m2.Y());
	list->push_back(m2.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m3.X());
	list->push_back(m3.Y());
	list->push_back(m3.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m4.X());
	list->push_back(m4.Y());
	list->push_back(m4.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m5.X());
	list->push_back(m5.Y());
	list->push_back(m5.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m6.X());
	list->push_back(m6.Y());
	list->push_back(m6.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m7.X());
	list->push_back(m7.Y());
	list->push_back(m7.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m8.X());
	list->push_back(m8.Y());
	list->push_back(m8.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(m9.X());
	list->push_back(m9.Y());
	list->push_back(m9.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(ma.X());
	list->push_back(ma.Y());
	list->push_back(ma.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(mb.X());
	list->push_back(mb.Y());
	list->push_back(mb.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(mc.X());
	list->push_back(mc.Y());
	list->push_back(mc.Z());
}

void CCuboid::OnApplyProperties()
{
	CCuboid* new_object = new CCuboid(m_pos, m_x, m_y, m_z, m_title.c_str());
	wxGetApp().StartHistory("Edit Cuboid");
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(this);
	wxGetApp().EndHistory();
	wxGetApp().m_marked_list->Clear();
	wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();
}

bool CCuboid::GetScaleAboutPoint(double* pos, double* x_axis, double* y_axis)
{
	extract(m_pos.Location(), pos);
	extract(m_pos.XDirection(), x_axis);
	extract(m_pos.YDirection(), y_axis);
	return true;
}