// HXml.cpp
#include "stdafx.h"

#include "HXml.h"

HXml::HXml(TiXmlElement* pElem):m_element(*pElem){
}

HXml::~HXml(){
}

const wxChar* HXml::GetShortString(void)const
{
	return Ctt(m_element.Value());
}

HeeksObj *HXml::MakeACopy(void)const{
	HXml *new_object = new HXml(*this);
	return new_object;
}

void HXml::GetProperties(std::list<Property *> *list){

	HeeksObj::GetProperties(list);
}

void HXml::WriteXML(TiXmlElement *root)
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

