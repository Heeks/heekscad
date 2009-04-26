// Group.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Group.h"
#include "../tinyxml/tinyxml.h"

void CGroup::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( "Group" );  
	root->LinkEndChild( element );
	WriteBaseXML(element);
}

// static member function
HeeksObj* CGroup::ReadFromXMLElement(TiXmlElement* pElem)
{
	CGroup* new_object = new CGroup;
	new_object->ReadBaseXML(pElem);

	return (ObjList*)new_object;
}
