// PictureFrame.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

// A scrolled window for showing an image.
class PictureFrame: public wxScrolledWindow
{   
protected:
	wxBitmap m_bitmap;

public:
    PictureFrame(wxWindow* parent, const wxBitmap& b);
    void OnPaint(wxPaintEvent &event);

private:
    DECLARE_EVENT_TABLE()
};

class PictureWindow: public wxWindow
{   
public:
	wxBitmap m_bitmap;
	bool m_bitmap_set;
	std::map<wxString, wxBitmap*> m_bitmaps;

    PictureWindow(wxWindow* parent, const wxSize& size);
    PictureWindow(wxWindow* parent, const wxBitmap& b);
	~PictureWindow();

    void OnPaint(wxPaintEvent &event);
	void SetPicture(const wxBitmap& b);
	void SetPicture(const wxString& filepath, long image_type);

private:
    DECLARE_EVENT_TABLE()
};
