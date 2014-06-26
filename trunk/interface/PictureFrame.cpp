// PictureFrame.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <stdafx.h>
#include "PictureFrame.h"

PictureFrame::PictureFrame(wxWindow* parent, const wxBitmap& b): wxScrolledWindow(parent), m_bitmap(b){
	SetScrollRate( 10, 10 );
	int w = m_bitmap.GetWidth();
	int h = m_bitmap.GetHeight();
	SetVirtualSize(w, h);
}

void PictureFrame::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.DrawBitmap(m_bitmap, 0,0, false);
}

BEGIN_EVENT_TABLE(PictureFrame,wxScrolledWindow)
    EVT_PAINT(PictureFrame::OnPaint)
END_EVENT_TABLE()

PictureWindow::PictureWindow(wxWindow* parent, const wxSize& size): wxWindow(parent, wxID_ANY, wxDefaultPosition, size), m_bitmap_set(false)
{
}

PictureWindow::PictureWindow(wxWindow* parent, const wxBitmap& b): wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(b.GetWidth(), b.GetHeight())), m_bitmap(b), m_bitmap_set(true)
{
}

PictureWindow::~PictureWindow()
{
	for(std::map<wxString, wxBitmap*>::iterator It = m_bitmaps.begin(); It != m_bitmaps.end(); It++)
	{
		wxBitmap* bitmap = It->second;
		delete bitmap;
	}
}

void PictureWindow::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	if(m_bitmap_set)
	{
		dc.DrawBitmap(m_bitmap, 0,0, false);
	}
}

void PictureWindow::SetPicture(const wxBitmap& b)
{
	m_bitmap = b;
	if(!m_bitmap_set)m_bitmap_set = true;
	Refresh();
}

void PictureWindow::SetPicture(const wxString& filepath, long image_type)
{
	std::map<wxString, wxBitmap*>::iterator FindIt = m_bitmaps.find(filepath);

	wxBitmap* bitmap = NULL;
	if(FindIt == m_bitmaps.end())
	{
		bitmap = new wxBitmap(wxImage(filepath, image_type));
	}
	else
	{
		bitmap = FindIt->second;
	}

	SetPicture(*bitmap);
}

BEGIN_EVENT_TABLE(PictureWindow,wxWindow)
    EVT_PAINT(PictureWindow::OnPaint)
END_EVENT_TABLE()
