// History.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class HeeksObj;

#include "../interface/Tool.h"

class History: public Undoable{
protected:
	std::list<Undoable *>::iterator m_curpos;
	std::list<Undoable *> m_undoables;

private:
	History *sub_history;
	int level;

	virtual void SetAsNewPos(std::list<Undoable *>::iterator &){}
	virtual void RemoveAsNewPosIfEqual(std::list<Undoable *>::iterator &){}

public:
	History(int Level);
	virtual ~History(void);

	// Undoable's virtual functions
	const wxChar* GetTitle(){return _T("");}
	void Run(bool redo);
	void RollBack();

	bool InternalRollBack(void);
	bool InternalRollForward(void);
	bool CanUndo(void);
	bool CanRedo(void);
	void DoUndoable(Undoable *);
	void Add(Undoable *);
	void StartHistory();
	bool EndHistory(void);
	int size(void){return m_undoables.size();}
	void Clear(std::list<Undoable *>::iterator FromIt);
	void ClearFromFront(void);
	void ClearFromCurPos(void);
};

class MainHistory: public History{
private:
	std::list<Undoable *>::iterator as_new_pos;
	bool as_new_pos_exists;
	bool as_new_when_at_list_start;

	// History virtual function
	void SetAsNewPos(std::list<Undoable *>::iterator &It){as_new_pos = It; as_new_pos_exists = true;}
	void RemoveAsNewPosIfEqual(std::list<Undoable *>::iterator &It);

public:
	MainHistory(void): History(0){as_new_pos_exists = false; as_new_when_at_list_start = true;}
	~MainHistory(void){}

	bool IsModified(void);
	void SetLikeNewFile(void);
	void DoUndoable(Undoable *);
	void SetAsModified();
};
