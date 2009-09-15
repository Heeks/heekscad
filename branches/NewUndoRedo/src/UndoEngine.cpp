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

UndoEvent::UndoEvent(EventType type, ObjList* parent, HeeksObj* object, HeeksObj* oldobject)
{
	m_type = type;
	m_parent = parent;
	m_object = object;
	m_oldobject = oldobject;
}

UndoEngine::UndoEngine(ObjList* tree)
{
	m_tree.m_tree = tree;
	m_oldtree.m_tree = new ObjList();
	m_oldtree.m_tree->m_id = tree->m_id;
	m_level = -1;
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

void UndoEngine::RecalculateMapsRecursive(std::map<HeeksObjId,HeeksObj*> &treemap, ObjList* obj)
{
	HeeksObj *new_obj = obj->GetFirstChild();
	while(new_obj)
	{
		HeeksObjId id = GetHeeksObjId(new_obj);
		treemap[id] = new_obj;

		ObjList* new_list = dynamic_cast<ObjList*>(new_obj);
		if(new_list && new_list->DescendForUndo())
			RecalculateMapsRecursive(treemap,new_list);
		new_obj = obj->GetNextChild();
	}

}

void UndoEngine::RecalculateMaps()
{
	m_tree.m_treemap.clear();
	m_oldtree.m_treemap.clear();

	m_tree.m_treemap[GetHeeksObjId(m_tree.m_tree)] = m_tree.m_tree;
	m_oldtree.m_treemap[GetHeeksObjId(m_tree.m_tree)] = m_oldtree.m_tree;

	RecalculateMapsRecursive(m_tree.m_treemap,m_tree.m_tree);
	RecalculateMapsRecursive(m_oldtree.m_treemap,m_oldtree.m_tree);
}

void UndoEngine::GetModifications(std::vector<UndoEvent> &ret,ObjList* newtree, ObjList* oldtree)
{
	//Add the parents to the map really quick
	RecalculateMaps();
	GetModificationsRecursive(ret,newtree,oldtree);
}


void UndoEngine::GetModificationsRecursive(std::vector<UndoEvent> &ret,ObjList* newtree, ObjList* oldtree)
{
	std::set<HeeksObjId> new_children;
	std::set<HeeksObjId> old_children;
	std::map<HeeksObjId,HeeksObj*> new_children_map;
	std::map<HeeksObjId,HeeksObj*> old_children_map;


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
		m_tree.m_treemap[*it] = obj;
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
			if(obj->IsDifferent(old_children_map[*it]))
			{
				HeeksObj* copy = obj->MakeACopyWithID();
				ret.push_back(UndoEvent(EventTypeModified,newtree,copy,old_children_map[*it]));
				m_oldtree.m_treemap[*it] = copy;
			}
			else
			{
				ObjList* newlist = dynamic_cast<ObjList*>(obj);
				ObjList* oldlist = dynamic_cast<ObjList*>(old_children_map[*it]);
				if(newlist && newlist->DescendForUndo())
				{
					GetModificationsRecursive(ret,newlist,oldlist);
				}
			}
		}
	}

	for(it = old_children.begin(); it != old_children.end(); it++)
	{
		HeeksObj* obj = old_children_map[*it];
		if(new_children.find(*it) == new_children.end())
			ret.push_back(UndoEvent(EventTypeRemove,newtree,obj->MakeACopyWithID()));
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
	RecalculateMaps();
	for(size_t i=0; i < events.size(); i++)
	{
		UndoEvent evt = events[i];
		switch(evt.m_type)
		{
			case EventTypeAdd:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Remove(tree->m_treemap[GetHeeksObjId(evt.m_object)]);
				break;
			case EventTypeModified:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Remove(tree->m_treemap[GetHeeksObjId(evt.m_object)]);
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(evt.m_oldobject->MakeACopyWithID(),NULL);
				break;
			case EventTypeRemove:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(evt.m_object->MakeACopyWithID(),NULL);
				break;
		}
	}
}

void UndoEngine::DoEvents(std::vector<UndoEvent> &events, EventTreeMap* tree)
{
	RecalculateMaps();
	for(size_t i=0; i < events.size(); i++)
	{
		UndoEvent evt = events[i];
		switch(evt.m_type)
		{
			case EventTypeAdd:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(evt.m_object->MakeACopyWithID(),NULL);
				break;
			case EventTypeModified:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Remove(tree->m_treemap[GetHeeksObjId(evt.m_oldobject)]);
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(evt.m_object->MakeACopyWithID(),NULL);
				break;
			case EventTypeRemove:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Remove(tree->m_treemap[GetHeeksObjId(evt.m_object)]);
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
		if(m_level < 0)
			m_level = 0;
		m_events.resize(m_level+1);
		m_events[m_level--] = events;

		UndoEvents(events, &m_tree);
		PrintTrees();
		return;
	}

	if(m_level>=0)
	{
		if(m_level >= m_events.size())
			m_level = m_events.size()-1;
		UndoEvents(m_events[m_level],&m_tree);
		UndoEvents(m_events[m_level--],&m_oldtree);
		PrintTrees();
	}
}

void UndoEngine::Redo()
{
	if(m_level != -1 && (unsigned)(m_level + 1) >= m_events.size())
		return;

	m_level++;
	DoEvents(m_events[m_level],&m_oldtree);
	DoEvents(m_events[m_level],&m_tree);

	PrintTrees();
}

void UndoEngine::CreateUndoPoint()
{
	std::vector<UndoEvent> events = GetModifications();	
	if(events.size() == 0)
		return;

	if(m_level < 0)
		m_level = 0;

	m_events.resize(m_level+1);
	m_events[m_level] = events;
	DoEvents(m_events[m_level],&m_oldtree);
	m_level++;

	PrintTrees();
}

void debugprint(std::string s);

void UndoEngine::PrintTrees()
{
	std::stringstream cstr;
    cstr << "OldTree: " << endl;
	PrintTree(m_oldtree.m_tree,cstr,0);
    cstr << "NewTree: " << endl;
	PrintTree(m_tree.m_tree,cstr,0);
    debugprint(cstr.str());
    cstr.clear();
}

void tab(std::stringstream &cstr, int tabs)
{
	for(int i=0; i < tabs; i++)
		cstr << "     ";
}

void UndoEngine::PrintTree(HeeksObj *tree, std::stringstream &cstr,int level)
{
	tab(cstr,level);
    cstr << "ID: " << tree->m_id << endl;
	tab(cstr,level);
	cstr << "Type: " << tree->GetTypeString() << endl;

	ObjList* list = dynamic_cast<ObjList*>(tree);
	if(list&&list->DescendForUndo())
	{
		HeeksObj* child = list->GetFirstChild();
		while(child)
		{
			PrintTree(child,cstr,level+1);
			child = list->GetNextChild();
		}
	}
}