// ShapeData.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once


class CShape;

enum SolidTypeEnum{
	SOLID_TYPE_UNKNOWN, // probably not a primitive solid anymore
	SOLID_TYPE_SPHERE,
	SOLID_TYPE_CYLINDER,
	SOLID_TYPE_CUBOID,
	SOLID_TYPE_CONE
};

// used for reading and writing to the XML file
class CShapeData{
public:
	CShapeData();
	CShapeData(CShape* shape);

	int m_id;
	bool m_visible;
	wxString m_title;
	SolidTypeEnum m_solid_type;

	TiXmlElement m_xml_element;
	std::list<int> m_vertex_ids;
	std::list<int> m_edge_ids;
	std::list<int> m_face_ids;

	void SetShape(CShape* shape);
};

