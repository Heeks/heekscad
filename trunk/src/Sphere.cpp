// Sphere.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sphere.h"
#include "../interface/PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "Gripper.h"
#include "MarkedList.h"

CSphere::CSphere(const gp_Pnt& pos, double radius, const wxChar* title, const HeeksColor& col):CSolid(BRepPrimAPI_MakeSphere(pos, radius), title, col), m_pos(pos), m_radius(radius)
{
}

CSphere::CSphere(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col):CSolid(solid, title, col), m_pos(0, 0, 0), m_radius(0.0)
{
}

CSphere::CSphere( const CSphere & rhs ) : CSolid(rhs)
{
    *this = rhs;    // Call the assignment operator.
}

CSphere & CSphere::operator= ( const CSphere & rhs )
{
    if (this != &rhs)
    {
        m_pos = rhs.m_pos;
        m_radius = rhs.m_radius;

        CSolid::operator=( rhs );
    }

    return(*this);
}

HeeksObj *CSphere::MakeACopy(void)const
{
	return new CSphere(*this);
}

bool CSphere::IsDifferent(HeeksObj *other)
{
	CSphere* sphere = (CSphere*)other;
	if(sphere->m_pos.Distance(m_pos) > wxGetApp().m_geom_tol || sphere->m_radius != m_radius)
		return true;
	return CShape::IsDifferent(other);
}

static void on_set_centre(const double *pos, HeeksObj* object){
	((CSphere*)object)->m_pos = make_point(pos);
}

static void on_set_radius(double value, HeeksObj* object){
	((CSphere*)object)->m_radius = value;
}

CShape* CSphere::MakeTransformedShape(const gp_Trsf &mat)
{
	gp_Pnt new_pos = m_pos.Transformed(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	double new_radius = fabs(m_radius * scale);
	return new CSphere(new_pos, new_radius, m_title.c_str(), m_color);
}

wxString CSphere::StretchedName(){ return _("Ellipsoid");}

void CSphere::GetProperties(std::list<Property *> *list)
{
	double pos[3];
	extract(m_pos, pos);
	list->push_back(new PropertyVertex(_("centre"), pos, this, on_set_centre));
	list->push_back(new PropertyLength(_("radius"), m_radius, this, on_set_radius));

	CSolid::GetProperties(list);
}

void CSphere::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	list->push_back(GripData(GripperTypeTranslate,m_pos.X(),m_pos.Y(),m_pos.Z(),NULL));
	list->push_back(GripData(GripperTypeScale,m_pos.X() + m_radius,m_pos.Y(),m_pos.Z(),NULL));
}

void CSphere::OnApplyProperties()
{
	CSphere* new_object = new CSphere(m_pos, m_radius, m_title.c_str(), m_color);
	new_object->CopyIDsFrom(this);
	Owner()->Add(new_object, NULL);
	Owner()->Remove(this);
	if(wxGetApp().m_marked_list->ObjectMarked(this))
	{
		wxGetApp().m_marked_list->Remove(this,false);
		wxGetApp().m_marked_list->Add(new_object, true);
	}
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
