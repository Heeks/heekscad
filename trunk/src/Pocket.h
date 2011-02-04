// Pocket.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "DynamicSolid.h"

class HPocket: public DynamicSolid{
public:
	double m_length;
	CSketch* m_sketch;

	HPocket(double length);
	HPocket();

	const wxChar* GetTypeString(void)const{return _("Pocket");}
	int GetType()const{return PocketSolidType;}
	HeeksObj *MakeACopy(void)const{ return new HPocket(*this);}
	void glCommands(bool select, bool marked, bool no_color);
	void GetProperties(std::list<Property *> *list);
	void WriteXML(TiXmlNode *root);
	bool UsesID(){return true;}
	long GetMarkingMask()const{return MARKING_FILTER_POCKETSOLID;}
	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	static void PocketSketch(CSketch*,double length);
	bool IsDifferent(HeeksObj* other);
	void ReloadPointers();
	gp_Trsf GetTransform();

	void Update();
};

