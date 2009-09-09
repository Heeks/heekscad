// UndoEngine.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "../interface/ObjList.h"

//This code attempt to determine what can be undone/redone by analyzing the heekscad graph and an 
//internal graph of the previous state. 

typedef std::pair<int,int> HeeksObjId;

enum EventType
{
	EventTypeAdd,
	EventTypeRemove,
	EventTypeModified
};

class UndoEvent
{
	EventType m_type;
	ObjList* m_parent;
	HeeksObj* m_object;
public:
	UndoEvent(EventType,ObjList*,HeeksObj*);
};

class UndoEngine
{
private:
	ObjList* m_oldtree;
	ObjList* m_tree;
	std::vector<std::vector<UndoEvent> > m_events;
	int m_undopoint;
public:
	UndoEngine(ObjList* tree);
	~UndoEngine();

	void ClearHistory();

protected:
	std::vector<UndoEvent> GetModifications();
	void GetModifications(std::vector<UndoEvent> &ret,ObjList* newtree, ObjList* oldtree);
	HeeksObjId GetHeeksObjId(HeeksObj*);
	HeeksObj* GetHeeksObj(HeeksObjId);
};

UndoEvent::UndoEvent(EventType type, ObjList* parent, HeeksObj* object)
{
	m_type = type;
	m_parent = parent;
	m_object = object;
}

UndoEngine::UndoEngine(ObjList* tree)
{
	m_tree = tree;
	m_oldtree = new ObjList();
	m_undopoint = 0;
}

UndoEngine::~UndoEngine()
{
	delete m_oldtree;
}

void UndoEngine::ClearHistory()
{
	delete m_oldtree;
	m_oldtree = new ObjList();
	m_events.clear();
	m_undopoint = 0;
}

HeeksObjId UndoEngine::GetHeeksObjId(HeeksObj* obj)
{
	return HeeksObjId(obj->GetType(),obj->m_id);
}

HeeksObj* UndoEngine::GetHeeksObj(HeeksObjId id)
{
	//TODO: get from heekscad for adds. removes from our db somehow
	return NULL;
}

std::vector<UndoEvent> UndoEngine::GetModifications()
{
	std::vector<UndoEvent> ret;
	GetModifications(ret,m_tree,m_oldtree);
	return ret;
}

void UndoEngine::GetModifications(std::vector<UndoEvent> &ret,ObjList* newtree, ObjList* oldtree)
{
	std::set<HeeksObjId> new_children;
	std::set<HeeksObjId> old_children;
	HeeksObj *new_obj = newtree->GetFirstChild();
	while(new_obj)
	{
		new_children.insert(GetHeeksObjId(new_obj));
		new_obj = newtree->GetNextChild();
	}

	HeeksObj *old_obj = oldtree->GetFirstChild();
	while(old_obj)
	{
		old_children.insert(GetHeeksObjId(new_obj));
		old_obj = oldtree->GetNextChild();
	}

	std::set<HeeksObjId>::iterator it;
	for(it = new_children.begin(); it != new_children.end(); it++)
	{
		if(old_children.find(*it) == old_children.end())
		{
			//TODO, this is actually tricky, when an item is added, it may be added in multiple places in the tree
			//we must make sure that multiple pointers get setup to this object, we also must deep copy
			ret.push_back(UndoEvent(EventTypeAdd,oldtree,GetHeeksObj(*it)));
		}
		else
		{
			//TODO: check if item is modified, if it is an objlist, descend
		}
	}

	for(it = old_children.begin(); it != old_children.end(); it++)
	{
		if(new_children.find(*it) == new_children.end())
			ret.push_back(UndoEvent(EventTypeRemove,newtree,GetHeeksObj(*it)));
	}
}