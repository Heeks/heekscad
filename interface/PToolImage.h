// PToolImage.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#ifndef HEEKSPLUGIN
#error PToolImage is for use only with plugins!
#endif

#include <wx/image.h>

class PToolImage: public wxImage{
public:

    PToolImage(const wxString& name, bool full_path_given = false);

	static int GetBitmapSize();
};

