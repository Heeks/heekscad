// ToolImage.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "ToolImage.h"

#ifdef HEEKSCAD
float ToolImage::m_button_scale = 0.25;
const int ToolImage::full_size = 96;
const int ToolImage::default_bitmap_size = 24;

#ifdef WIN32
ToolImage::ToolImage(const wxString& name):wxImage(wxGetApp().GetExeFolder() + _T("/bitmaps/") + name + _T(".png"), wxBITMAP_TYPE_PNG)
#else
ToolImage::ToolImage(const wxString& name):wxImage(wxGetApp().GetExeFolder() + _T("/../share/heekscad/bitmaps/") + name + _T(".png"), wxBITMAP_TYPE_PNG)
#endif
#else
ToolImage::ToolImage(const wxString& name):wxImage(theApp.GetDllFolder() + _T("/bitmaps/") + name + _T(".png"), wxBITMAP_TYPE_PNG)
#endif
{
	int width = GetWidth();
	int height = GetHeight();
#ifdef HEEKSCAD
	float button_scale = m_button_scale;
#else
	float button_scale = heeksCAD->GetToolImageButtonScale();
#endif
	int new_width = (int)(button_scale * width);
	int new_height = (int)(button_scale * height);
	Rescale(new_width, new_height);
}

int ToolImage::GetBitmapSize()
{
#ifdef HEEKSCAD
	return (int)(m_button_scale * full_size);
#else
	return heeksCAD->GetToolImageBitmapSize();
#endif
}

#ifdef HEEKSCAD
void ToolImage::SetBitmapSize(int size)
{
	m_button_scale = (float)size / (float)full_size;
}
#endif
