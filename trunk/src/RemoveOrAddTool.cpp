// RemoveOrAddTool.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "RemoveOrAddTool.h"
#include "../interface/HeeksObj.h"
#include "MarkedList.h"

RemoveOrAddTool::RemoveOrAddTool(HeeksObj *object, HeeksObj *owner, HeeksObj *prev_object) : m_belongs_to_owner(false)
{
	m_object = object;
	m_owner = owner;
	m_prev_object = prev_object;
}

RemoveOrAddTool::~RemoveOrAddTool()
{
	if(m_owner == NULL)return;
	if(!m_belongs_to_owner)delete m_object;
}

static wxString string_for_GetTitle;

const wxChar* AddObjectTool::GetTitle()
{
	string_for_GetTitle.assign(_("Add "));
	string_for_GetTitle.append(m_object->GetShortStringOrTypeString());
	return string_for_GetTitle.c_str();
}

void RemoveOrAddTool::Add()
{

	if (wxGetApp().m_doing_rollback && (m_owner == NULL))
	{
		m_owner = NULL;
	}

	if (m_owner == NULL)
	{
		wxMessageBox(_T("Can't Have NULL owner!"));
		return;
	}

	m_owner->Add(m_object, m_prev_object);

	wxGetApp().WasAdded(m_object);
	wxGetApp().WasModified(m_owner);

	m_belongs_to_owner = true;
}

void RemoveOrAddTool::Remove()
{
	while (m_object->Owner())
	{
		m_owner = m_object->Owner();
		m_object->Owner()->Remove(m_object);
		wxGetApp().WasRemoved(m_object);
		wxGetApp().WasModified(m_owner);
		m_object->RemoveOwner(m_object->Owner());
	}
	m_belongs_to_owner = false;
}

void AddObjectTool::Run()
{
	Add();
}

void AddObjectTool::RollBack()
{
	Remove();
}

RemoveObjectTool::RemoveObjectTool(HeeksObj *object):RemoveOrAddTool(object, NULL, NULL)
{
	if(object)m_owner = object->Owner();
	else m_owner = NULL;
}

void RemoveObjectTool::Run()
{
	Remove();
}

void RemoveObjectTool::RollBack()
{
	Add();
}


ManyRemoveOrAddTool::~ManyRemoveOrAddTool()
{
	if(!m_belongs_to_owner){
		std::list<HeeksObj*>::iterator It;
		for(It = m_objects.begin(); It != m_objects.end(); It++){
			HeeksObj* object = *It;
            delete object;
		}
	}
}

void ManyRemoveOrAddTool::Add()
{
	if (m_owner == NULL)
	{
		wxMessageBox(_T("Can't have NULL owner!"));
		return;
	}

	std::list<HeeksObj*>::iterator It;
	for(It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		m_owner->Add(object, NULL);
	}

	wxGetApp().WereAdded(m_objects);
	wxGetApp().WasModified(m_owner);

	m_belongs_to_owner = true;
}

void ManyRemoveOrAddTool::Remove()
{
	std::list<HeeksObj*>::iterator It;
	for(It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		if(m_owner)
			m_owner->Remove(object);
		wxGetApp().m_marked_list->Remove(object, false);
	}

	wxGetApp().WereRemoved(m_objects);
	wxGetApp().WasModified(m_owner);
	for(It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		object->RemoveOwners();
	}

	m_belongs_to_owner = false;
}

const wxChar* AddObjectsTool::GetTitle()
{
	return _("Add Objects");
}

void AddObjectsTool::Run()
{
	Add();
}

void AddObjectsTool::RollBack()
{
	Remove();
}

const wxChar* RemoveObjectsTool::GetTitle()
{
	return _("Remove Objects");
}

void RemoveObjectsTool::Run()
{
	Remove();
}

void RemoveObjectsTool::RollBack()
{
	Add();
}

void ChangeOwnerTool::Run()
{
	// to do
}

void ChangeOwnerTool::RollBack()
{
	// to do
}

ManyChangeOwnerTool::ManyChangeOwnerTool(const std::list<HeeksObj*> &list, HeeksObj* new_owner): m_objects(list), m_new_owner(new_owner)
{
	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		m_prev_owners.push_back(object->Owner());
	}
}

void ManyChangeOwnerTool::Run()
{
	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		object->Owner()->Remove(object);
		wxGetApp().m_marked_list->Remove(object, false);
	}

	wxGetApp().WereRemoved(m_objects);
	wxGetApp().WereModified(m_prev_owners);

	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		object->RemoveOwner(object->Owner());
		m_new_owner->Add(object, NULL);
	}

	wxGetApp().WereAdded(m_objects);
	wxGetApp().WasModified(m_new_owner);
}

void ManyChangeOwnerTool::RollBack()
{
	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		m_new_owner->Remove(object);
		wxGetApp().m_marked_list->Remove(object, false);
	}

	wxGetApp().WereRemoved(m_objects);
	wxGetApp().WasModified(m_new_owner);

	std::list<HeeksObj*>::iterator PrevIt = m_objects.begin();
	for(std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++, PrevIt++){
		HeeksObj* object = *It;
		HeeksObj* prev_owner = *PrevIt;
		prev_owner->Add(object, NULL);
	}

	wxGetApp().WereAdded(m_objects);
	wxGetApp().WereModified(m_prev_owners);
}
