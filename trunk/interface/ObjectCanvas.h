// ObjectCanvas.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

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
