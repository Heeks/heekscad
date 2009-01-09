// Sphere.cpp

#include "stdafx.h"
#include "Sphere.h"
#include <BRepPrimAPI_MakeSphere.hxx>
#include <gp_Trsf.hxx>
#include "PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "Gripper.h"
#include "MarkedList.h"
#include "../tinyxml/tinyxml.h"
#include "HeeksCAD.h"

CSphere::CSphere(const gp_Pnt& pos, double radius, const wxChar* title, const HeeksColor& col):CSolid(BRepPrimAPI_MakeSphere(pos, radius), title, col), m_pos(pos), m_radius(radius)
{
}

CSphere::CSphere(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col):CSolid(solid, title, col), m_pos(0, 0, 0), m_radius(0.0)
{
}

HeeksObj *CSphere::MakeACopy(void)const
{
	return new CSphere(*this);
}

static void on_set_centre(const gp_Pnt &vt, HeeksObj* object){
	((CSphere*)object)->m_pos = vt;
}

static void on_set_radius(double value, HeeksObj* object){
	((CSphere*)object)->m_radius = value;
}

bool CSphere::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	gp_Pnt new_pos = m_pos.Transformed(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	double new_radius = fabs(m_radius * scale);
	CSphere* new_object = new CSphere(new_pos, new_radius, m_title.c_str(), m_color);
	wxGetApp().AddUndoably(new_object, m_owner, NULL);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().DeleteUndoably(this);
	return true;
}

void CSphere::GetProperties(std::list<Property *> *list)
{
	CSolid::GetProperties(list);

	list->push_back(new PropertyVertex(_("centre"), m_pos, this, on_set_centre));
	list->push_back(new PropertyDouble(_("radius"), m_radius, this, on_set_radius));
}

void CSphere::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	list->push_back(GripperTypeTranslate);
	list->push_back(m_pos.X());
	list->push_back(m_pos.Y());
	list->push_back(m_pos.Z());
	list->push_back(GripperTypeScale);
	list->push_back(m_pos.X() + m_radius);
	list->push_back(m_pos.Y());
	list->push_back(m_pos.Z());
}

void CSphere::OnApplyProperties()
{
	CSphere* new_object = new CSphere(m_pos, m_radius, m_title.c_str(), m_color);
	wxGetApp().StartHistory();
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(this);
	wxGetApp().EndHistory();
	wxGetApp().m_marked_list->Clear();
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object);
	wxGetApp().Repaint();
}

bool CSphere::GetCentrePoint(double* pos)
{
	extract(m_pos, pos);
	return true;
}

bool CSphere::GetScaleAboutMatrix(double *m)
{
	gp_Trsf mat;
	mat.SetTranslationPart(gp_Vec(m_pos.XYZ()));
	extract(mat, m);
	return true;
}

void CSphere::SetXMLElement(TiXmlElement* element)
{
	element->SetDoubleAttribute("px", m_pos.X());
	element->SetDoubleAttribute("py", m_pos.Y());
	element->SetDoubleAttribute("pz", m_pos.Z());
	element->SetDoubleAttribute("r", m_radius);

	CSolid::SetXMLElement(element);
}

void CSphere::SetFromXMLElement(TiXmlElement* pElem)
{
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "px"){m_pos.SetX(a->DoubleValue());}
		else if(name == "py"){m_pos.SetY(a->DoubleValue());}
		else if(name == "pz"){m_pos.SetZ(a->DoubleValue());}
		else if(name == "r"){m_radius = a->DoubleValue();}
	}

	CSolid::SetFromXMLElement(pElem);
}
