// Tool.h

#pragma once

class wxBitmap;

class Tool
{
	public:
	virtual ~Tool(){}

	virtual void Run() = 0;
	virtual const char* GetTitle() = 0;
	virtual const char* GetToolTip(){return "";}
	virtual bool Undoable(){return false;}
	virtual void RollBack(){};
	virtual bool Disabled(){return false;}
	virtual bool Checked(){return false;}
	virtual bool IsAToolList() {return false;}
	virtual wxBitmap* Bitmap(){return NULL;} // 32x32 picture for toolbar button
};
