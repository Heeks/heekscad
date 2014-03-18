// HXml.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HXml.h"

HXml::HXml(TiXmlElement* pElem):m_element(*pElem){
}

HXml::~HXml(){
}

HeeksObj *HXml::MakeACopy(void)const{
	HXml *new_object = new HXml(*this);
	return new_object;
}

void HXml::GetProperties(std::list<Property *> *list){

	HeeksObj::GetProperties(list);
}

const wxBitmap &HXml::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/xml.png")));
	return *icon;
}

void HXml::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( m_element );
	root->LinkEndChild( element );  
	//WriteBaseXML(element);
}

// static member function
HeeksObj* HXml::ReadFromXMLElement(TiXmlElement* pElem)
{
	HXml* new_object = new HXml(pElem);
	//new_object->ReadBaseXML(pElem);

	return new_object;
}

