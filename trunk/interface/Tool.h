// Tool.h

#pragma once

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
	virtual const wxChar* GetToolTip(){return _T("");}
	virtual bool Undoable(){return false;}
	virtual void RollBack(){};
	virtual bool Disabled(){return false;}
	virtual bool Checked(){return false;}
	virtual bool IsAToolList() {return false;}
	virtual wxString BitmapPath(){return _T("");}
	virtual wxBitmap* Bitmap(){if((m_bitmap == NULL || (m_icon_size != ToolImage::GetBitmapSize())) && BitmapPath().Len() > 0){delete m_bitmap; m_bitmap = new wxBitmap(ToolImage(BitmapPath())); m_icon_size = ToolImage::GetBitmapSize();}return m_bitmap;}
};
