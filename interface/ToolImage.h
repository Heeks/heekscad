// ToolImage.h

#pragma once

class ToolImage: public wxImage{
public:
#ifdef HEEKSCAD
	static float m_button_scale;
	static const int full_size;
	static const int default_bitmap_size;
#endif

	ToolImage(const wxString& name);

	static int GetBitmapSize();
#ifdef HEEKSCAD
	static void SetBitmapSize(int size);
#endif
};

