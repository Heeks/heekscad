// ObjList.h

#pragma once

#include "HeeksObj.h"

class ObjList : public HeeksObj
{
protected:
	std::list<HeeksObj*> m_objects;
	std::list<HeeksObj*>::iterator LoopIt;
	std::list<std::list<HeeksObj*>::iterator> LoopItStack;
	std::vector<HeeksObj*> m_index_list; // for quick performance of GetAtIndex();
	bool m_index_list_valid;

	void Clear();
	void recalculate_index_list();

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
	HeeksObj* GetAtIndex(int index);
	int GetNumChildren();
	bool CanAdd(HeeksObj* object){return true;}
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Remove(HeeksObj* object);
	void KillGLLists(void);
	void WriteXML(TiXmlElement *root);
	void ReadXMLChildren(TiXmlElement* root);
};
