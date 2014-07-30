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

CSphere::CSphere(const gp_Pnt& pos, double radius, const wxChar* title, const HeeksColor& col, float opacity):CSolid(BRepPrimAPI_MakeSphere(pos, radius), title, col, opacity), m_pos(pos), m_radius(radius)
{
}

CSphere::CSphere(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col, float opacity):CSolid(solid, title, col, opacity), m_pos(0, 0, 0), m_radius(0.0)
{
}

CSphere::CSphere( const CSphere & rhs ) : CSolid(rhs)
{
    m_pos = rhs.m_pos;
    m_radius = rhs.m_radius;
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

const wxBitmap &CSphere::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/sphere.png")));
	return *icon;
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

void CSphere::MakeTransformedShape(const gp_Trsf &mat)
{
	m_pos.Transform(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	m_radius = fabs(m_radius * scale);
	m_shape = BRepPrimAPI_MakeSphere(m_pos, m_radius).Shape();
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
	CSphere* new_object = new CSphere(m_pos, m_radius, m_title.c_str(), m_color, m_opacity);
	new_object->CopyIDsFrom(this);
#ifdef MULTIPLE_OWNERS
	HEEKSOBJ_OWNER->Add(new_object, NULL);
	HEEKSOBJ_OWNER->Remove(this);
#else
	m_owner->Add(new_object, NULL);
	m_owner->Remove(this);
#endif
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
