// Pad.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/ObjList.h"

class CPad: public ObjList{
public:
	const wxChar* GetTypeString(void)const{return _("Pad");}
	int GetType()const{return PadType;}
	HeeksObj *MakeACopy(void)const{ return new CPad(*this);}
	wxString GetIcon(){return wxGetApp().GetResFolder() + _T("/icons/group");}
	void WriteXML(TiXmlNode *root);
	bool UsesID(){return true;}
	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};

