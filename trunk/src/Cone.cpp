// Cone.cpp

#include "stdafx.h"
#include "Cone.h"
#include <BRepPrimAPI_MakeCone.hxx>
#include "PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "Gripper.h"
#include "MarkedList.h"
#include "../tinyxml/tinyxml.h"

CCone::CCone(const gp_Ax2& pos, double r1, double r2, double height, const wxChar* title, const HeeksColor& col):CSolid(BRepPrimAPI_MakeCone(pos, r1, r2, height), title, col), m_pos(pos), m_r1(r1), m_r2(r2), m_height(height)
{
}

CCone::CCone(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col):CSolid(solid, title, col), m_pos(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), m_r1(0.0), m_r2(0.0), m_height(0.0)
{
}

HeeksObj *CCone::MakeACopy(void)const
{
	return new CCone(*this);
}

static void on_set_centre(const gp_Pnt &vt, HeeksObj* object){
	gp_Trsf mat;
	mat.SetTranslation ( gp_Vec ( ((CCone*)object)->m_pos.Location(), vt ) );
	((CCone*)object)->m_pos.Transform(mat);
}

static void on_set_r1(double value, HeeksObj* object){
	((CCone*)object)->m_r1 = value;
}

static void on_set_r2(double value, HeeksObj* object){
	((CCone*)object)->m_r2 = value;
}

static void on_set_height(double value, HeeksObj* object){
	((CCone*)object)->m_height = value;
}

bool CCone::ModifyByMatrix(const double *m){
	gp_Trsf mat = make_matrix(m);
	gp_Ax2 new_pos = m_pos.Transformed(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	double new_r1 = fabs(m_r1 * scale);
	double new_r2 = fabs(m_r2 * scale);
	double new_height = fabs(m_height * scale);
	CCone* new_object = new CCone(new_pos, new_r1, new_r2, new_height, m_title.c_str(), m_color);
	wxGetApp().AddUndoably(new_object, m_owner, NULL);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().DeleteUndoably(this);

	return true;
}

void CCone::GetProperties(std::list<Property *> *list)
{
	CSolid::GetProperties(list);

	list->push_back(new PropertyVertex(_("centre pos"), m_pos.Location(), this, on_set_centre));
	list->push_back(new PropertyVertex(_("direction"), gp_Pnt(m_pos.Direction().XYZ()), NULL));
	list->push_back(new PropertyVertex(_("x direction"), gp_Pnt(m_pos.XDirection().XYZ()), NULL));
	list->push_back(new PropertyDouble(_("r1"), m_r1, this, on_set_r1));
	list->push_back(new PropertyDouble(_("r2"), m_r2, this, on_set_r2));
	list->push_back(new PropertyDouble(_("height"), m_height, this, on_set_height));
}

void CCone::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_r1);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt py(o.XYZ() + m_pos.YDirection().XYZ() * m_r1);
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_r2 + z_dir.XYZ() * m_height);
	gp_Pnt pmx(o.XYZ() + m_pos.XDirection().XYZ() * (-m_r1));
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);
	gp_Pnt pxz(o.XYZ() + m_pos.XDirection().XYZ() * m_r2 + z_dir.XYZ() * m_height);
	list->push_back(GripperTypeTranslate);
	list->push_back(o.X());
	list->push_back(o.Y());
	list->push_back(o.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(px.X());
	list->push_back(px.Y());
	list->push_back(px.Z());
	list->push_back(GripperTypeObjectScaleZ);
	list->push_back(pz.X());
	list->push_back(pz.Y());
	list->push_back(pz.Z());
	list->push_back(GripperTypeStretch);
	list->push_back(pxz.X());
	list->push_back(pxz.Y());
	list->push_back(pxz.Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(py.X());
	list->push_back(py.Y());
	list->push_back(py.Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(pmx.X());
	list->push_back(pmx.Y());
	list->push_back(pmx.Z());
}

void CCone::OnApplyProperties()
{
	CCone* new_object = new CCone(m_pos, m_r1, m_r2, m_height, m_title.c_str(), m_color);
	wxGetApp().StartHistory();
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(this);
	wxGetApp().EndHistory();
	wxGetApp().m_marked_list->Clear();
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();
}


bool CCone::GetScaleAboutMatrix(double *m)
{
	gp_Trsf mat = make_matrix(m_pos.Location(), m_pos.XDirection(), m_pos.YDirection());
	extract(mat, m);
	return true;
}

bool CCone::Stretch2(const double *p, const double* shift, gp_Ax2& new_pos, double& new_r1, double& new_r2, double& new_height)
{
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_r1);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt py(o.XYZ() + m_pos.YDirection().XYZ() * m_r1);
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_r2 + z_dir.XYZ() * m_height);
	gp_Pnt pmxz(o.XYZ() + m_pos.XDirection().XYZ() * (-m_r2) + z_dir.XYZ() * m_height);
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);
	gp_Pnt pxz(o.XYZ() + m_pos.XDirection().XYZ() * m_r2 + z_dir.XYZ() * m_height);

	bool make_a_new_cone = false;

	if(px.IsEqual(vp, wxGetApp().m_geom_tol)){
		px = px.XYZ() + vshift.XYZ();
		double new_x = gp_Vec(px.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		double new_y = gp_Vec(px.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		new_r1 = sqrt(new_x * new_x + new_y * new_y);
		if(fabs(new_r1 - m_r2) > 0.000000001){
			make_a_new_cone = true;
		}
	}
	else if(pxz.IsEqual(vp, wxGetApp().m_geom_tol)){
		pxz = pxz.XYZ() + vshift.XYZ();
		double new_x = gp_Vec(pxz.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		double new_y = gp_Vec(pxz.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		new_r2 = sqrt(new_x * new_x + new_y * new_y);
		if(fabs(m_r1 - new_r2) > 0.000000001){
			make_a_new_cone = true;
		}
	}
	else if(pz.IsEqual(vp, wxGetApp().m_geom_tol)){
		pz = pz.XYZ() + vshift.XYZ();
		new_height = gp_Vec(pz.XYZ()) * gp_Vec(z_dir) - gp_Vec(o.XYZ()) * gp_Vec(z_dir);
		if(new_height > 0){
			make_a_new_cone = true;
		}
	}

	return make_a_new_cone;
}

bool CCone::Stretch(const double *p, const double* shift)
{
	gp_Ax2 new_pos = m_pos;
	double new_r1 = m_r1;
	double new_r2 = m_r2;
	double new_height = m_height;

	bool make_a_new_cone = Stretch2(p, shift, new_pos, new_r1, new_r2, new_height);

	if(make_a_new_cone)
	{
		CCone* new_object = new CCone(new_pos, new_r1, new_r2, new_height, m_title.c_str(), m_color);
		wxGetApp().StartHistory();
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().DeleteUndoably(this);
		wxGetApp().EndHistory();
		wxGetApp().m_marked_list->Clear();
		wxGetApp().m_marked_list->Add(new_object);
	}

	return true;
}

bool CCone::StretchTemporary(const double *p, const double* shift)
{
	return false;
}

void CCone::SetXMLElement(TiXmlElement* element)
{
	const gp_Pnt& l = m_pos.Location();
	element->SetDoubleAttribute("lx", l.X());
	element->SetDoubleAttribute("ly", l.Y());
	element->SetDoubleAttribute("lz", l.Z());

	const gp_Dir& d = m_pos.Direction();
	element->SetDoubleAttribute("dx", d.X());
	element->SetDoubleAttribute("dy", d.Y());
	element->SetDoubleAttribute("dz", d.Z());

	const gp_Dir& x = m_pos.XDirection();
	element->SetDoubleAttribute("xx", x.X());
	element->SetDoubleAttribute("xy", x.Y());
	element->SetDoubleAttribute("xz", x.Z());

	element->SetDoubleAttribute("r1", m_r1);
	element->SetDoubleAttribute("r2", m_r2);
	element->SetDoubleAttribute("h", m_height);

	CSolid::SetXMLElement(element);
}

void CCone::SetFromXMLElement(TiXmlElement* pElem)
{
	double l[3] = {0, 0, 0};
	double d[3] = {0, 0, 1};
	double x[3] = {1, 0, 0};

	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "lx")	 {l[0] = a->DoubleValue();}
		else if(name == "ly"){l[1] = a->DoubleValue();}
		else if(name == "lz"){l[2] = a->DoubleValue();}

		else if(name == "dx"){d[0] = a->DoubleValue();}
		else if(name == "dy"){d[1] = a->DoubleValue();}
		else if(name == "dz"){d[2] = a->DoubleValue();}

		else if(name == "xx"){x[0] = a->DoubleValue();}
		else if(name == "xy"){x[1] = a->DoubleValue();}
		else if(name == "xz"){x[2] = a->DoubleValue();}

		else if(name == "r1"){m_r1 = a->DoubleValue();}
		else if(name == "r2"){m_r2 = a->DoubleValue();}
		else if(name == "h"){m_height = a->DoubleValue();}
	}

	m_pos = gp_Ax2(make_point(l), make_vector(d), make_vector(x));

	CSolid::SetFromXMLElement(pElem);
}
