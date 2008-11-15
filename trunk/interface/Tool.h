// Tool.h

#pragma once

class wxBitmap;

class Tool
{
	public:
	virtual ~Tool(){}

	virtual void Run() = 0;
	virtual const wxChar* GetTitle() = 0;
	virtual const wxChar* GetToolTip(){return _T("");}
	virtual bool Undoable(){return false;}
	virtual void RollBack(){};
	virtual bool Disabled(){return false;}
	virtual bool Checked(){return false;}
	virtual bool IsAToolList() {return false;}
	virtual wxBitmap* Bitmap(){return NULL;}
};
