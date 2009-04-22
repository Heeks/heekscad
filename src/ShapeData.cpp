// ShapeData.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "ShapeData.h"
#include "Solid.h"

CShapeData::CShapeData(): m_xml_element("")
{
	m_id = -1;
	m_solid_type = SOLID_TYPE_UNKNOWN;
	m_visible = true;
}

CShapeData::CShapeData(CShape* shape): m_xml_element("")
{
	m_id = shape->m_id;
	m_title = shape->m_title;
	m_visible = shape->m_visible;
	m_solid_type = SOLID_TYPE_UNKNOWN;
	if(shape->GetType() == SolidType)m_solid_type = ((CSolid*)shape)->GetSolidType();
	shape->SetXMLElement(&m_xml_element);
}

void CShapeData::SetShape(CShape* shape)
{
	if(m_id != -1)shape->SetID(m_id);
	if(m_title.length() > 0)shape->m_title = m_title;
	shape->m_visible = m_visible;
	shape->SetFromXMLElement(&m_xml_element);
}

