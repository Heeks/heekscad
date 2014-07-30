// ToolList.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HeeksObj.h"
#include "ToolList.h"

void ToolList::Add(Tool *t){
	m_tool_list.push_back(t);
}

void ToolList::Add(std::list<Tool*>& tools)
{
	std::list<Tool*>::iterator It;
	for (It=tools.begin();It!=tools.end();It++)
		m_tool_list.push_back(*It);
}

void ToolList::Clear(){
	std::list<Tool*>::iterator It;
	for(It = m_tool_list.begin(); It != m_tool_list.end(); It++){
		delete (*It);
	}
	m_tool_list.clear();
}
