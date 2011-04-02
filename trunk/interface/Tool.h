// Tool.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class wxString;
class wxBitmap;

#include "ToolImage.h"

class Tool
{
	public:
	wxBitmap* m_bitmap;
	int m_icon_size;

	Tool():m_bitmap(NULL){}
	virtual ~Tool(){if(m_bitmap)delete m_bitmap;}

	virtual void Run() = 0;
	virtual const wxChar* GetTitle() = 0;
	virtual const wxChar* GetToolTip(){return GetTitle();}
	virtual bool Disabled(){return false;}
	virtual bool Checked(){return false;}
	virtual bool IsAToolList() {return false;}
	virtual bool IsSeparator() const {return false;}
	virtual wxString BitmapPath(){return _T("");}
	virtual wxBitmap* Bitmap(){if(m_bitmap && m_icon_size == ToolImage::GetBitmapSize())return m_bitmap; wxString str = BitmapPath(); if(str.Len() > 0){delete m_bitmap; m_bitmap = new wxBitmap(ToolImage(str)); m_icon_size = ToolImage::GetBitmapSize();}return m_bitmap;}
	virtual bool CallChangedOnRun(){return true;}
};
