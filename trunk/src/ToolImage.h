// ToolImage.h

#pragma once

class ToolImage: public wxImage{
public:
	static float m_button_scale;
	static const int full_size;
	static const int default_bitmap_size;

	ToolImage(const wxString& name);

	static int GetBitmapSize();
	static void SetBitmapSize(int size);
};