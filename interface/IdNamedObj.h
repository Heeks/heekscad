// IdNamedObj.h
/*
 * Copyright (c) 2013, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#pragma once

#include "HeeksObj.h"

class IdNamedObj: public HeeksObj
{
public:
	wxString m_title;
	bool m_title_made_from_id;

	IdNamedObj():m_title_made_from_id(true){}

	// HeeksObj's virtual functions
	void WriteBaseXML(TiXmlElement *element);
	void ReadBaseXML(TiXmlElement* element);
    const wxChar* GetShortString(void)const;
    bool CanEditString(void)const{return true;}
    void OnEditString(const wxChar* str);
};
