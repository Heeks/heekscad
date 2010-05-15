// PictureFrame.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "PictureFrame.h"

PictureFrame::PictureFrame(wxWindow* parent, const wxBitmap& b): wxScrolledWindow(parent), m_bitmap(b){
	SetScrollRate( 10, 10 );
	SetVirtualSize(m_bitmap.GetWidth(), m_bitmap.GetHeight());
}

void PictureFrame::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.DrawBitmap(m_bitmap, 0,0, true);
}

BEGIN_EVENT_TABLE(PictureFrame,wxScrolledWindow)
    EVT_PAINT(PictureFrame::OnPaint)
END_EVENT_TABLE()
