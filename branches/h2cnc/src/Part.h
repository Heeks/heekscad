// Part.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "DynamicSolid.h"

class CPart: public DynamicSolid{
public:

	CPart();
	~CPart();

	const wxChar* GetTypeString(void)const{return _("Part");}
	int GetType()const{return PartType;}
	HeeksObj *MakeACopy(void)const{ return new CPart(*this);}
	long GetMarkingMask()const{return MARKING_FILTER_PART;}
	void glCommands(bool select, bool marked, bool no_color);
	void Update();
};

