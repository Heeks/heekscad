// ToolImage.cpp

#include "stdafx.h"
#include "ToolImage.h"

float ToolImage::m_button_scale = 0.25;
const int ToolImage::full_size = 96;
const int ToolImage::default_bitmap_size = 24;

#ifdef HEEKSCAD
ToolImage::ToolImage(const wxString& name):wxImage(wxGetApp().GetExeFolder() + _T("/bitmaps/") + name + _T(".png"), wxBITMAP_TYPE_PNG)
#else
ToolImage::ToolImage(const wxString& name):wxImage(theApp.GetDllFolder() + _T("/bitmaps/") + name + _T(".png"), wxBITMAP_TYPE_PNG)
#endif
{
	int width = GetWidth();
	int height = GetHeight();
	int new_width = (int)(m_button_scale * width);
	int new_height = (int)(m_button_scale * height);
	Rescale(new_width, new_height);
}

int ToolImage::GetBitmapSize()
{
	return (int)(m_button_scale * full_size);
}

void ToolImage::SetBitmapSize(int size)
{
	m_button_scale = (float)size / (float)full_size;
}
