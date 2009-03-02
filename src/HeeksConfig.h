// HeeksConfig.h
/*
 * Copyright (c) 2009, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */
#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>

class HeeksConfig: public wxConfig
{
public:
	HeeksConfig():wxConfig(_T("HeeksCAD")){}
	~HeeksConfig(){}
};
