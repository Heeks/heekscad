// ToolList.h

#pragma once

#include "Tool.h"

class ToolList: public Tool{
private:
	std::list<Tool*>::iterator LoopIt;

public:
	std::string m_title;
	std::list<Tool*> m_tool_list;

	ToolList(const char *t): m_title(t){}
	ToolList(std::list<Tool*>& tools, const char *t): m_tool_list(tools), m_title(t){}
	~ToolList(){Clear();}

	// Tool's virtual functions
	const char* GetTitle(){return m_title.c_str();}
	void Run(){};
	bool IsAToolList(){return true;}

	// member functions
	void Add(Tool *t);
	void Add(std::list<Tool*>& tools);
	void Clear();
};
