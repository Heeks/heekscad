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
protected:
	wxBitmap m_bitmap;
	bool m_bitmap_set;

public:
    PictureWindow(wxWindow* parent, const wxSize& size);
    PictureWindow(wxWindow* parent, const wxBitmap& b);
    void OnPaint(wxPaintEvent &event);
	void SetPicture(const wxBitmap& b);
	void SetPicture(wxBitmap** b, const wxString& filepath, long image_type);

private:
    DECLARE_EVENT_TABLE()
};
