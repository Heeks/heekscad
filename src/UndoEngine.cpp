// UndoEngine.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "../interface/ObjList.h"
#include "UndoEngine.h"
#include "../interface/TransientObject.h"

#ifdef USE_UNDO_ENGINE
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
	m_undo_events.clear();
	m_redo_events.clear();
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
		//Check the owner for debugging
		//TODO: This is fubar, WTF is causing objects to get copied with no owners
		//only happens when they objects are > 2 levels deep. 
#ifdef MULTIPLE_OWNERS
		bool found=false;
		HeeksObj* owner = new_obj->GetFirstOwner();
		while(owner)
		{
			if(owner == obj)
				found=true;
			owner = new_obj->GetNextOwner();
		}
		if(!found)
		{
			new_obj->AddOwner(obj);
		}
#else
		if(new_obj->m_owner != obj)new_obj->m_owner = obj;
#endif
		HeeksObjId id = GetHeeksObjId(new_obj);
		treemap[id] = new_obj;
		if(new_obj->IsList())
		{
			ObjList* new_list = (ObjList*)new_obj;

			//Always descend when generating the map
			RecalculateMapsRecursive(treemap,new_list);
		}
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
			if(!obj->GetSkipForUndo() && obj->IsDifferent(old_children_map[*it]))
			{
				HeeksObj* copy = obj->MakeACopyWithID();
				ret.push_back(UndoEvent(EventTypeModified,newtree,copy,old_children_map[*it]));
				m_oldtree.m_treemap[*it] = copy;
			}
			else
			{
				if(obj->IsList())
				{
					ObjList* newlist = (ObjList*)obj;
					ObjList* oldlist = (ObjList*)old_children_map[*it];
					if(newlist->DescendForUndo())
					{
						GetModificationsRecursive(ret,newlist,oldlist);
					}
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

	//DealWithTransients();
	wxGetApp().ClearTransients();
}

void UndoEngine::DealWithTransients(std::map<HeeksObjId,HeeksObj*> &treemap)
{
	std::map<HeeksObj*,std::list<HeeksObj*> >& map = wxGetApp().GetTransients();
	
	std::map<HeeksObj*,std::list<HeeksObj*> >::iterator it;
	std::list<HeeksObj*> needupdate;
	for(it = map.begin(); it!= map.end(); it++)
	{
		TransientObject *tobj = (TransientObject*)(*it).first;

		std::list<HeeksObj*>::iterator it2;
		for(it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
		{
			HeeksObj* obj = (HeeksObj*)(*it2);
			std::map<HeeksObjId,HeeksObj*>::iterator it3 = treemap.find(GetHeeksObjId(obj));
			if(it3 == treemap.end())
			{
				HeeksObj* nobj = obj->MakeACopyWithID();
#ifdef MULTIPLE_OWNERS
				nobj->RemoveOwners();
#else
				nobj->m_owner = NULL;
#endif
				needupdate.push_back(nobj);
				treemap[GetHeeksObjId(nobj)] = nobj;
#ifdef MULTIPLE_OWNERS
				tobj->Owner()->Add(nobj,NULL);
#else
				tobj->m_owner->Add(nobj,NULL);
#endif

			}
			else
			{
				needupdate.push_back((*it3).second);
#ifdef MULTIPLE_OWNERS
				tobj->Owner()->Add((*it3).second, NULL);
#else
				tobj->m_owner->Add((*it3).second, NULL);
#endif
			}
		}

#ifdef MULTIPLE_OWNERS
		tobj->Owner()->Remove(tobj);
#else
		tobj->m_owner->Remove(tobj);
#endif
		//delete tobj;
	}

	//Deal with the quick pointer problem
	std::list<HeeksObj*>::iterator it2;
	for(it2 = needupdate.begin(); it2 != needupdate.end(); it2++)
	{
		HeeksObj *obj = *it2;
#ifdef MULTIPLE_OWNERS
		HeeksObj *owner = obj->GetFirstOwner();
		while(owner)
		{
			owner->ReloadPointers();
			owner = obj->GetNextOwner();
		}
#else
		if(obj->m_owner)obj->m_owner->ReloadPointers();
#endif
		obj->ReloadPointers();
	}

	wxGetApp().ClearTransients();	
}

bool UndoEngine::IsModified()
{
	std::vector<UndoEvent> events = GetModifications();
	return events.size()>0 || m_undo_events.size() > 0;
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
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->ReloadPointers();
				break;
			case EventTypeModified:
				{
					tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Remove(tree->m_treemap[GetHeeksObjId(evt.m_object)]);
					HeeksObj* new_obj = evt.m_oldobject->MakeACopyWithID();
					tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(new_obj,NULL);
					tree->m_treemap[GetHeeksObjId(new_obj)] = new_obj;
				}
				break;
			case EventTypeRemove:
				{
					HeeksObj* new_obj = evt.m_object->MakeACopyWithID();
					tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(new_obj,NULL);
					tree->m_treemap[GetHeeksObjId(new_obj)] = new_obj;
					new_obj->ReloadPointers();
#ifdef MULTIPLE_OWNERS
					new_obj->Owner()->ReloadPointers();
#else
					new_obj->m_owner->ReloadPointers();
#endif
				}
				break;
		}
		DealWithTransients(tree->m_treemap);
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
				{
					HeeksObj* new_obj = evt.m_object->MakeACopyWithID();
					tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(new_obj,NULL);
					tree->m_treemap[GetHeeksObjId(new_obj)] = new_obj;
					new_obj->ReloadPointers();
#ifdef MULTIPLE_OWNERS
					new_obj->Owner()->ReloadPointers();
#else
					new_obj->m_owner->ReloadPointers();
#endif
				}
				break;
			case EventTypeModified:
				{
					tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Remove(tree->m_treemap[GetHeeksObjId(evt.m_oldobject)]);
					HeeksObj* new_obj = evt.m_object->MakeACopyWithID();
					tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Add(new_obj,NULL);
					tree->m_treemap[GetHeeksObjId(new_obj)] = new_obj;
				}
				break;
			case EventTypeRemove:
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->Remove(tree->m_treemap[GetHeeksObjId(evt.m_object)]);
				tree->m_treemap[GetHeeksObjId(evt.m_parent)]->ReloadPointers();
				break;
		}
		DealWithTransients(tree->m_treemap);
	}
}


void UndoEngine::SetLikeNewFile()
{
	CreateUndoPoint();
	m_undo_events.clear();
	m_redo_events.clear();
}

void UndoEngine::Undo()
{
	//First try to rollback to the last savepoint
	std::vector<UndoEvent> events = GetModifications();	
	if(events.size() > 0)
	{
		UndoEvents(events, &m_tree);
		m_redo_events.push_back(events);
		PrintTrees();
		return;
	}

	if(m_undo_events.size())
	{
		events = m_undo_events.back();
		m_undo_events.pop_back();
		UndoEvents(events,&m_tree);
		UndoEvents(events,&m_oldtree);
		m_redo_events.push_back(events);
		PrintTrees();
	}
}

void UndoEngine::Redo()
{
	if(!m_redo_events.size())
		return;

	DoEvents(m_redo_events.back(),&m_oldtree);
	DoEvents(m_redo_events.back(),&m_tree);

	m_undo_events.push_back(m_redo_events.back());
	m_redo_events.pop_back();

	PrintTrees();
}

void UndoEngine::CreateUndoPoint()
{
	std::vector<UndoEvent> events = GetModifications();	
	if(events.size() == 0)
		return;

	m_undo_events.push_back(events);
	m_redo_events.clear();
	DoEvents(events,&m_oldtree);

	PrintTrees();
}

void debugprint(std::string s)
{
	wxLogDebug(wxString(s.c_str(),wxConvUTF8));
}

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
	cstr << "Type: " << wxString(tree->GetTypeString()).mb_str() << endl;
	tab(cstr,level);
	cstr << "Location: " << tree << endl;

	if(tree->IsList())
	{
		ObjList* list = (ObjList*)tree;
		if(list->DescendForUndo())
		{
			HeeksObj* child = list->GetFirstChild();
			while(child)
			{
				PrintTree(child,cstr,level+1);
				child = list->GetNextChild();
			}
		}
	}
}

#endif