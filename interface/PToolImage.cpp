// PToolImage.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

//PToolImage - rename ToolImage so that code in libheekscnc doesn't try to use the
// version of ToolImage that's in HeeksCAD. Whatever the issue is, it is exposed
// when cmake is used as the build system

#include "stdafx.h"
#include "PToolImage.h"

PToolImage::PToolImage(const wxString& name, bool full_path_given):wxImage(full_path_given?name:(theApp.GetResFolder() + _T("/bitmaps/") + name + _T(".png")), wxBITMAP_TYPE_PNG)
{
	int width = GetWidth();
	int height = GetHeight();
	float button_scale = heeksCAD->GetToolImageButtonScale();
	int new_width = (int)(button_scale * width);
	int new_height = (int)(button_scale * height);
	Rescale(new_width, new_height);
}

int PToolImage::GetBitmapSize()
{
	return heeksCAD->GetToolImageBitmapSize();
}
