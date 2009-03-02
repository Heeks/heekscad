// History.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#ifdef WIN32
#pragma once
#endif

class HeeksObj;

#include "../interface/Tool.h"

class History: public Tool{
protected:
	std::list<Tool *>::iterator m_curpos;
	std::list<Tool *> m_tools;

private:
	History *sub_history;
	int level;

	virtual void SetAsNewPos(std::list<Tool *>::iterator &){}
	virtual void RemoveAsNewPosIfEqual(std::list<Tool *>::iterator &){}

public:
	History(int Level);
	virtual ~History(void);

	// Tool's virtual functions
	const wxChar* GetTitle(){return _T("");}
	void Run();
	void RollBack();

	bool InternalRollBack(void);
	bool InternalRollForward(void);
	void DoToolUndoably(Tool *);
	void Add(Tool *);
	void StartHistory();
	bool EndHistory(void);
	int size(void){return m_tools.size();}
	void Clear(std::list<Tool *>::iterator FromIt);
	void ClearFromFront(void);
	void ClearFromCurPos(void);
};

class MainHistory: public History{
private:
	std::list<Tool *>::iterator as_new_pos;
	bool as_new_pos_exists;
	bool as_new_when_at_list_start;

	// History virtual function
	void SetAsNewPos(std::list<Tool *>::iterator &It){as_new_pos = It; as_new_pos_exists = true;}
	void RemoveAsNewPosIfEqual(std::list<Tool *>::iterator &It);

public:
	MainHistory(void): History(0){as_new_pos_exists = false; as_new_when_at_list_start = true;}
	~MainHistory(void){}

	bool IsModified(void);
	void SetLikeNewFile(void);
	void DoToolUndoably(Tool *);
	void SetAsModified();
};
