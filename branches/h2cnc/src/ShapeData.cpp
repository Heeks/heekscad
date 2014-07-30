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

	for(HeeksObj* object = shape->m_faces->GetFirstChild(); object; object = shape->m_faces->GetNextChild())
	{
		m_face_ids.push_back(object->m_id);
	}

	for(HeeksObj* object = shape->m_edges->GetFirstChild(); object; object = shape->m_edges->GetNextChild())
	{
		m_edge_ids.push_back(object->m_id);
	}

	for(HeeksObj* object = shape->m_vertices->GetFirstChild(); object; object = shape->m_vertices->GetNextChild())
	{
		m_vertex_ids.push_back(object->m_id);
	}

}

void CShapeData::SetShape(CShape* shape)
{
	if(m_id != -1)shape->SetID(m_id);
	if(m_title.length() > 0)shape->m_title = m_title;
	shape->m_visible = m_visible;
	shape->SetFromXMLElement(&m_xml_element);

	{
		std::list<int>::iterator It = m_face_ids.begin();
		for(HeeksObj* object = shape->m_faces->GetFirstChild(); object && It != m_face_ids.end(); object = shape->m_faces->GetNextChild(), It++)
		{
			object->SetID(*It);
		}
	}

	{
		std::list<int>::iterator It = m_edge_ids.begin();
		for(HeeksObj* object = shape->m_edges->GetFirstChild(); object && It != m_edge_ids.end(); object = shape->m_edges->GetNextChild(), It++)
		{
			object->SetID(*It);
		}
	}

	{
		std::list<int>::iterator It = m_vertex_ids.begin();
		for(HeeksObj* object = shape->m_vertices->GetFirstChild(); object && It != m_vertex_ids.end(); object = shape->m_vertices->GetNextChild(), It++)
		{
			object->SetID(*It);
		}
	}
}

