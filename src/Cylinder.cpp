// Cylinder.cpp

#include "stdafx.h"
#include "Cylinder.h"
#include <BRepPrimAPI_MakeCylinder.hxx>
#include "PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "Gripper.h"
#include "MarkedList.h"
#include "../tinyxml/tinyxml.h"

wxIcon* CCylinder::m_icon = NULL;

CCylinder::CCylinder(const gp_Ax2& pos, double radius, double height, const wxChar* title, const HeeksColor& col):m_pos(pos), m_radius(radius), m_height(height), CSolid(BRepPrimAPI_MakeCylinder(pos, radius, height), title, col)
{
}

CCylinder::CCylinder(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col):CSolid(solid, title, col), m_pos(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), m_radius(0.0), m_height(0.0)
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

bool CCylinder::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	gp_Ax2 new_pos = m_pos.Transformed(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	double new_radius = fabs(m_radius * scale);
	double new_height = fabs(m_height * scale);
	CCylinder* new_object = new CCylinder(new_pos, new_radius, new_height, m_title.c_str(), m_color);
	wxGetApp().AddUndoably(new_object, m_owner, NULL);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().DeleteUndoably(this);
	return true;
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
	CCylinder* new_object = new CCylinder(m_pos, m_radius, m_height, m_title.c_str(), m_color);
	wxGetApp().StartHistory(_T("Edit Cylinder"));
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(this);
	wxGetApp().EndHistory();
	wxGetApp().m_marked_list->Clear();
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();
}


bool CCylinder::GetScaleAboutMatrix(double *m)
{
	gp_Trsf mat = make_matrix(m_pos.Location(), m_pos.XDirection(), m_pos.YDirection());
	extract(mat, m);
	return true;
}

bool CCylinder::Stretch(const double *p, const double* shift)
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
		double new_x = gp_Vec(px.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		double new_y = gp_Vec(px.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		make_a_new_cylinder = true;
		m_radius = sqrt(new_x * new_x + new_y * new_y);
	}
	else if(pz.IsEqual(vp, wxGetApp().m_geom_tol)){
		pz = pz.XYZ() + vshift.XYZ();
		double new_height = gp_Vec(pz.XYZ()) * gp_Vec(z_dir) - gp_Vec(o.XYZ()) * gp_Vec(z_dir);
		if(new_height > 0){
			make_a_new_cylinder = true;
			m_height = new_height;
		}
	}

	if(make_a_new_cylinder)
	{
		CCylinder* new_object = new CCylinder(m_pos, m_radius, m_height, m_title.c_str(), m_color);
		wxGetApp().StartHistory(_T("Stretch Cylinder"));
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().DeleteUndoably(this);
		wxGetApp().EndHistory();
		wxGetApp().m_marked_list->Clear();
		wxGetApp().m_marked_list->Add(new_object);
	}

	return true;
}

void CCylinder::SetXMLElement(TiXmlElement* element)
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

	element->SetDoubleAttribute("r", m_radius);
	element->SetDoubleAttribute("h", m_height);

	CSolid::SetXMLElement(element);
}

void CCylinder::SetFromXMLElement(TiXmlElement* pElem)
{
	// get the attributes
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

		else if(name == "r"){m_radius = a->DoubleValue();}
		else if(name == "h"){m_height = a->DoubleValue();}
	}

	m_pos = gp_Ax2(make_point(l), make_vector(d), make_vector(x));

	CSolid::SetFromXMLElement(pElem);
}
