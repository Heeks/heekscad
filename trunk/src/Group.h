// Group.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/ObjList.h"

class CGroup: public ObjList{
	std::list<int> m_loaded_solid_ids; // only used during loading xml file

public:
	const wxChar* GetTypeString(void)const{return _("Group");}
	int GetType()const{return GroupType;}
	HeeksObj *MakeACopy(void)const{ return new CGroup(*this);}
#ifdef WIN32
	wxString GetIcon(){return wxGetApp().GetExeFolder() + _T("/icons/group");}
#else
	wxString GetIcon(){return wxGetApp().GetExeFolder() + _T("/../share/heekscad/icons/group");}
#endif
	void WriteXML(TiXmlNode *root);
	bool UsesID(){return true;}

	static void MoveSolidsToGroupsById(HeeksObj* object);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};

