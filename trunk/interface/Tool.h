// Tool.h

#pragma once

class Tool
{
	public:
	virtual ~Tool(){}

	virtual void Run() = 0;
	virtual const char* GetTitle() = 0;
	virtual bool Undoable(){return false;}
	virtual void RollBack(){};
	virtual bool Disabled(){return false;}
	virtual bool Checked(){return false;}
	virtual bool IsAToolList() {return false;}
};
