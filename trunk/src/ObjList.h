// ObjList.h

#pragma once

#include "../interface/HeeksObj.h"

class ObjList : public HeeksObj
{
protected:
	std::list<HeeksObj*> m_objects;
	std::list<HeeksObj*>::iterator LoopIt;
	std::list<std::list<HeeksObj*>::iterator> LoopItStack;

	void Clear();

public:
	ObjList(){}
	ObjList(const ObjList& objlist);
	virtual ~ObjList() {Clear();}

	const ObjList& operator=(const ObjList& objlist);

	void ClearUndoably(void);
	HeeksObj* MakeACopy(void) const;
	void GetBox(CBox &box);
	void glCommands(bool select, bool marked, bool no_color);
	HeeksObj* GetFirstChild();
	HeeksObj* GetNextChild();
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Remove(HeeksObj* object);
	void KillGLLists(void);
};
