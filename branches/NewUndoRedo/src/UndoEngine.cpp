// UndoEngine.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "../interface/ObjList.h"
#include "UndoEngine.h"

//This code attempt to determine what can be undone/redone by analyzing the heekscad graph and an 
//internal graph of the previous state. 

UndoEvent::UndoEvent(EventType type, ObjList* parent, HeeksObj* object)
{
	m_type = type;
	m_parent = parent;
	m_object = object;
}

UndoEngine::UndoEngine(ObjList* tree)
{
	m_tree.m_tree = tree;
	m_oldtree.m_tree = new ObjList();
	m_oldtree.m_tree->m_id = tree->m_id;
	m_level = 0;
}

UndoEngine::~UndoEngine()
{
	delete m_oldtree.m_tree;
}

void UndoEngine::ClearHistory()
{
	delete m_oldtree.m_tree;
	m_oldtree.m_tree = new ObjList();
	m_oldtree.m_tree->m_id = m_tree.m_tree->m_id;
	m_events.clear();
	m_level = 0;
}

HeeksObjId UndoEngine::GetHeeksObjId(HeeksObj* obj)
{
	return HeeksObjId(obj->GetType(),obj->m_id);
}

std::vector<UndoEvent> UndoEngine::GetModifications()
{
	std::vector<UndoEvent> ret;
	GetModifications(ret,m_tree.m_tree,m_oldtree.m_tree);
	return ret;
}

void UndoEngine::GetModifications(std::vector<UndoEvent> &ret,ObjList* newtree, ObjList* oldtree)
{
	std::set<HeeksObjId> new_children;
	std::set<HeeksObjId> old_children;
	std::map<HeeksObjId,HeeksObj*> new_children_map;
	std::map<HeeksObjId,HeeksObj*> old_children_map;
	m_tree.m_treemap.clear();
	m_oldtree.m_treemap.clear();

	//Add the parents to the map really quick
	m_tree.m_treemap[GetHeeksObjId(newtree)] = newtree;
	m_oldtree.m_treemap[GetHeeksObjId(newtree)] = oldtree;


	HeeksObj *new_obj = newtree->GetFirstChild();
	while(new_obj)
	{
		HeeksObjId id = GetHeeksObjId(new_obj);
		new_children.insert(id);
		new_children_map[id] = new_obj;
		new_obj = newtree->GetNextChild();
	}

	HeeksObj *old_obj = oldtree->GetFirstChild();
	while(old_obj)
	{
		HeeksObjId id = GetHeeksObjId(old_obj);
		old_children.insert(id);
		old_children_map[id] = old_obj;
		old_obj = oldtree->GetNextChild();
	}

	std::set<HeeksObjId>::iterator it;
	for(it = new_children.begin(); it != new_children.end(); it++)
	{
		HeeksObj* obj = new_children_map[*it];
		if(old_children.find(*it) == old_children.end())
		{
			//TODO, this is actually tricky, when an item is added, it may be added in multiple places in the tree
			//we must make sure that multiple pointers get setup to this object, we also must deep copy
			HeeksObj* copy = obj->MakeACopyWithID();
			ret.push_back(UndoEvent(EventTypeAdd,newtree,copy));
			m_oldtree.m_treemap[*it] = copy;
		}
		else
		{
			//TODO: check if item is modified, if it is an objlist, descend
		}
		m_tree.m_treemap[*it] = obj;
	}

	for(it = old_children.begin(); it != old_children.end(); it++)
	{
		HeeksObj* obj = old_children_map[*it];
		if(new_children.find(*it) == new_children.end())
			ret.push_back(UndoEvent(EventTypeRemove,newtree,obj));
		m_oldtree.m_treemap[*it] = obj;
	}
}

bool UndoEngine::IsModified()
{
	std::vector<UndoEvent> events = GetModifications();
	return events.size()>0;
}

void UndoEngine::UndoEvents(std::vector<UndoEvent> &events, EventTreeMap* tree)
{
	for(size_t i=0; i < events.size(); i++)
	{
		UndoEvent evt = events[i];
		switch(evt.m_type)
		{
			case EventTypeAdd:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Remove(tree->m_treemap[GetHeeksObjId(evt.m_object)]);
				break;
			default:
				break;
		}
	}
}

void UndoEngine::DoEvents(std::vector<UndoEvent> &events, EventTreeMap* tree)
{
	for(size_t i=0; i < events.size(); i++)
	{
		UndoEvent evt = events[i];
		switch(evt.m_type)
		{
			case EventTypeAdd:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(evt.m_object->MakeACopyWithID(),NULL);
				break;
			default:
				break;
		}
	}
}


void UndoEngine::SetLikeNewFile()
{
	//TODO: find all modifications, then set minimum undo level to current level
}

void UndoEngine::Undo()
{
	//First try to rollback to the last savepoint
	std::vector<UndoEvent> events = GetModifications();	
	if(events.size() > 0)
	{
		UndoEvents(events, &m_tree);
		return;
	}

	if(m_level>0)
	{
		UndoEvents(m_events[--m_level],&m_tree);
		UndoEvents(m_events[m_level],&m_oldtree);
	}
}

void UndoEngine::Redo()
{

}

void UndoEngine::CreateUndoPoint()
{
	std::vector<UndoEvent> events = GetModifications();	
	if(events.size() == 0)
		return;

	m_events.resize(m_level+1);
	m_events[m_level] = events;
	if(m_level>0)
		DoEvents(m_events[m_level-1],&m_oldtree);
	m_level++;
}