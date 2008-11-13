// Undo.cpp
#include "stdafx.h"
#include "../interface/HeeksObj.h"
#include "History.h"

History::History(int Level)
{
	sub_history = NULL;
	level = Level;
}

void History::Run()
{
	for(std::list<Tool *>::iterator It = m_tools.begin(); It != m_tools.end(); It++)
	{
		Tool *t = *It;
		t->Run();
	}
}

void History::RollBack(void)
{
	for(std::list<Tool *>::reverse_iterator It = m_tools.rbegin(); It != m_tools.rend(); It++)
	{
		Tool *t = *It;
		t->RollBack();
	}
}

bool History::InternalRollBack(void)
{
	if(m_tools.size() == 0)return false;
	if(m_curpos == m_tools.begin())return false;
	Tool *t;
	m_curpos--;
	t = *m_curpos;
	t->RollBack();
	return true;
}

bool History::InternalRollForward(void)
{
	if(m_tools.size() == 0)return false;
	if(m_curpos == m_tools.end())return false;
	Tool *t = *m_curpos;
	t->Run();
	m_curpos++;
	return true;
}

void History::StartHistory()
{
	if(sub_history)
	{
		sub_history->StartHistory();
	}
	else
	{
		sub_history = new History(level+1);
	}
}

bool History::EndHistory(void)
{
	if(sub_history)
	{
		if(sub_history->EndHistory())
		{
			if(sub_history->size()>0)Add(sub_history);
			else delete sub_history;
			sub_history = NULL;
		}
		return false;
	}
	else
	{
		return true;
	}
}

void History::DoToolUndoably(Tool *t)
{
	if(t == NULL)return;
	if(sub_history)sub_history->DoToolUndoably(t);
	else
	{
		t->Run();
		Add(t);
	}
}

void History::Add(Tool *t)
{
	if(m_tools.size() > 0 && m_curpos != m_tools.end())
	{
		ClearFromCurPos();
	}
	m_curpos = m_tools.end();
	m_tools.push_back(t);
	m_curpos = m_tools.end();
}

void History::Clear(std::list<Tool *>::iterator FromIt)
{
	if(m_tools.size() == 0)return;
	std::list<Tool *>::iterator It = m_tools.end();
	It--;
	for(;; It--)
	{
		RemoveAsNewPosIfEqual(It);
		Tool *t = *It;
		delete t;
		if(It == FromIt)break;
	}
	m_tools.erase(FromIt, m_tools.end());
}

void History::ClearFromFront(void)
{
	Clear(m_tools.begin());
}

void History::ClearFromCurPos(void)
{
	Clear(m_curpos);
}

History::~History(void)
{
	if(m_tools.size() == 0)return;
	std::list<Tool *>::iterator It;
	It = m_tools.end();
	It--;
	for(;; It--)
	{
		Tool *t = *It;
		delete t;
		if(It == m_tools.begin())break;
	}
}

bool MainHistory::IsModified(void)
{
	if(as_new_when_at_list_start)
	{
		if(size()==0)return false;
		if(m_curpos == m_tools.begin())return false;
		return true;
	}
	if(size() == 0)return true;
	if(as_new_pos_exists)
	{
		std::list<Tool *>::iterator TempIt = m_curpos;
		TempIt--;
		if(TempIt == as_new_pos)return false;
	}
	return true;
}

void MainHistory::RemoveAsNewPosIfEqual(std::list<Tool *>::iterator &It)
{
	if(as_new_pos_exists && as_new_pos == It)
	{
		as_new_pos_exists = false;
	}
}

void MainHistory::SetLikeNewFile(void)
{
	if(size() == 0)
	{
		as_new_when_at_list_start = true;
		as_new_pos_exists = false;
	}
	else
	{
		std::list<Tool *>::iterator TempIt = m_curpos;
		TempIt--;
		as_new_pos = TempIt;
		as_new_pos_exists = true;
		as_new_when_at_list_start = false;
	}
}

void MainHistory::DoToolUndoably(Tool *t)
{
	History::DoToolUndoably(t);
}

void MainHistory::SetAsModified()
{
	as_new_when_at_list_start = false;
	as_new_pos_exists = false;
}

