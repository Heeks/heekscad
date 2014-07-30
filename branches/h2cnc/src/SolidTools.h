// SolidTools.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../interface/Tool.h"

void GetSolidMenuTools(std::list<Tool*>* t_list);

class SaveSolids: public Tool {
public:

    virtual void Run();
	const wxChar* GetTitle(){return _("Save Solids");}
	wxString BitmapPath(){return _T("saveas");}
};
