// ShapeData.h

#pragma once

#include "../tinyxml/tinyxml.h"

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
	wxString m_title;
	SolidTypeEnum m_solid_type;

	TiXmlElement m_xml_element;
	std::map<int, int> m_edge_index; // map of index to id
	std::map<int, int> m_face_index; // map of index to id

	void SetShape(CShape* shape);
};