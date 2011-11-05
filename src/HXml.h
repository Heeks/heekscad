// HXml.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/HeeksObj.h"

class HXml: public HeeksObj{
public:
	TiXmlElement m_element;

	~HXml(void);
	HXml(TiXmlElement* pElem);

	// HeeksObj's virtual functions
	int GetType()const{return XmlType;}
	const wxChar* GetTypeString(void)const{return _("Xml");}
	HeeksObj *MakeACopy(void)const;
	const wxBitmap &GetIcon();
	void GetProperties(std::list<Property *> *list);
	void CopyFrom(const HeeksObj* object){operator=(*((HXml*)object));}
	void WriteXML(TiXmlNode *root);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

};
