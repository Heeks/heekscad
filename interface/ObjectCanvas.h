// ObjectCanvas.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "PictureFrame.h"

// base class for windows to appear in the "Properties" window
class ObjectCanvas: public wxScrolledWindow
{
public:
	ObjectCanvas(wxWindow* parent): wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE){}
	virtual ~ObjectCanvas(){}

public:
	virtual void SetWithObject(HeeksObj* object){}
	virtual void OnApply(){}
	virtual void OnCancel(){};
};

// a class which simply has a picture
class PictureCanvas: public ObjectCanvas
{
public:
	PictureCanvas::PictureCanvas(wxWindow* parent, const wxBitmap& bitmap) : ObjectCanvas(parent)
	{
		// just draw a picture
		PictureFrame* picture = new PictureFrame(this, bitmap);
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(picture, 1, wxGROW);
		SetSizer(sizer);
	}
};
