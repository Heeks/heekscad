// EndedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "EndedObject.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "../interface/PropertyVertex.h"
#include "HPoint.h"
#include "MarkedList.h"

EndedObject::EndedObject(void){
}

EndedObject::EndedObject(const EndedObject& e)
{
	operator=(e);
}

EndedObject::~EndedObject(){
}

const EndedObject& EndedObject::operator=(const EndedObject &b){
	HeeksObj::operator=(b);
	A = b.A;
	B = b.B;
	color = b.color;
	return *this;
}

HeeksObj* EndedObject::MakeACopyWithID()
{
	EndedObject* pnew = (EndedObject*)HeeksObj::MakeACopyWithID();
	return pnew;
}

bool EndedObject::IsDifferent(HeeksObj *other)
{
	EndedObject* eobj = (EndedObject*)other;
	if(eobj->A.Distance(A) > wxGetApp().m_geom_tol)
		return true;

	if(eobj->B.Distance(B) > wxGetApp().m_geom_tol)
		return true;

	if(color.COLORREF_color() != eobj->color.COLORREF_color())
		return true;

	return HeeksObj::IsDifferent(other);
}

void EndedObject::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A.Transform(mat);
	B.Transform(mat);
}

bool EndedObject::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(data == &A){
		A = vp.XYZ() + vshift.XYZ();
	}
	else if(data == &B){
		B = vp.XYZ() + vshift.XYZ();
	}
	return false;
}

void EndedObject::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	list->push_back(GripData(GripperTypeStretch,A.X(),A.Y(),A.Z(),&A));
	list->push_back(GripData(GripperTypeStretch,B.X(),B.Y(),B.Z(),&B));
}

bool EndedObject::GetStartPoint(double* pos)
{
	extract(A, pos);
	return true;
}

bool EndedObject::GetEndPoint(double* pos)
{
	extract(B, pos);
	return true;
}

void EndedObject::WriteBaseXML(TiXmlElement *element)
{
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("sx", A.X());
	element->SetDoubleAttribute("sy", A.Y());
	element->SetDoubleAttribute("sz", A.Z());
	element->SetDoubleAttribute("ex", B.X());
	element->SetDoubleAttribute("ey", B.Y());
	element->SetDoubleAttribute("ez", B.Z());

	HeeksObj::WriteBaseXML(element);
}

void EndedObject::ReadBaseXML(TiXmlElement* pElem)
{
	HeeksColor c;

	// get the attributes
	int att_col;
	double x;
	if(pElem->Attribute("col", &att_col))c = HeeksColor((long)att_col);
	if(pElem->Attribute("sx", &x))A.SetX(x);
	if(pElem->Attribute("sy", &x))A.SetY(x);
	if(pElem->Attribute("sz", &x))A.SetZ(x);
	if(pElem->Attribute("ex", &x))B.SetX(x);
	if(pElem->Attribute("ey", &x))B.SetY(x);
	if(pElem->Attribute("ez", &x))B.SetZ(x);

	else
	{
		// try the version where the points were children
		bool a_found = false;
		for(TiXmlElement* pElem2 = TiXmlHandle(pElem).FirstChildElement().Element(); pElem2;	pElem2 = pElem2->NextSiblingElement())
		{
			HeeksObj* object = wxGetApp().ReadXMLElement(pElem2);
			if(object->GetType() == PointType)
			{
				if(!a_found)
				{
					A = ((HPoint*)object)->m_p;
					a_found = true;
				}
				else
				{
					B = ((HPoint*)object)->m_p;
					delete object;
					break;
				}
			}
			delete object;
		}
	}

	HeeksObj::ReadBaseXML(pElem);
}
