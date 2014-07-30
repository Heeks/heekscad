// UndoEngine.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#ifdef USE_UNDO_ENGINE
typedef std::pair<int,int> HeeksObjId;
typedef std::pair<HeeksObjId,HeeksObjId> HeeksLinkId;

enum EventType
{
	EventTypeAdd,
	EventTypeRemove,
	EventTypeModified
};

class UndoEvent
{
public:
	EventType m_type;
	ObjList* m_parent;
	HeeksObj* m_object;
	HeeksObj* m_oldobject;

	UndoEvent(EventType,ObjList*,HeeksObj*);
	UndoEvent(EventType,ObjList*,HeeksObj*,HeeksObj*);
};

class EventTreeMap
{
public:
	ObjList* m_tree;
	std::map<HeeksObjId,HeeksObj*> m_treemap;
};

class UndoEngine
{
private:
	EventTreeMap m_oldtree;
	EventTreeMap m_tree;
	std::vector<std::vector<UndoEvent> > m_undo_events;
	std::vector<std::vector<UndoEvent> > m_redo_events;
public:
	UndoEngine(ObjList* tree);
	~UndoEngine();

	void ClearHistory();
	bool IsModified();
	void SetLikeNewFile();
	void Undo();
	void Redo();
	void CreateUndoPoint();

protected:
	std::vector<UndoEvent> GetModifications();
	void GetModifications(std::vector<UndoEvent> &ret,ObjList* newtree, ObjList* oldtree);
	void GetModificationsRecursive(std::vector<UndoEvent> &ret,ObjList* newtree, ObjList* oldtree);
	HeeksObjId GetHeeksObjId(HeeksObj*);
	void UndoEvents(std::vector<UndoEvent> &events, EventTreeMap* tree);
	void DoEvents(std::vector<UndoEvent> &events, EventTreeMap* tree);
	void RecalculateMaps();
	void RecalculateMapsRecursive(std::map<HeeksObjId,HeeksObj*> &treemap, ObjList* obj);

	void PrintTrees();
	void PrintTree(HeeksObj* tree, std::stringstream &cstr, int level);
	void DealWithTransients(std::map<HeeksObjId,HeeksObj*> &treemap);
};

#endif
