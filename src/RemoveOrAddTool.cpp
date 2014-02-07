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
	m_object->m_owner = m_owner;

	wxGetApp().WasAdded(m_object);
	wxGetApp().WasModified(m_owner);

	m_belongs_to_owner = true;
}

void RemoveOrAddTool::Remove()
{
	if (m_object->m_owner)
	{
		// to do multiple owners
		m_owner = m_object->m_owner;
		m_owner->Remove(m_object);
		wxGetApp().WasRemoved(m_object);
		wxGetApp().WasModified(m_owner);
		m_object->m_owner = NULL;
	}
	m_belongs_to_owner = false;
}

void AddObjectTool::Run(bool redo)
{
	Add();
}

void AddObjectTool::RollBack()
{
	Remove();
}

RemoveObjectTool::RemoveObjectTool(HeeksObj *object):RemoveOrAddTool(object, NULL, NULL)
{
	if(object)m_owner = object->m_owner;
	else m_owner = NULL;
}

void RemoveObjectTool::Run(bool redo)
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
		object->m_owner = m_owner;
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
		m_owner->Remove(object);
		wxGetApp().m_marked_list->Remove(object, false);
	}

	wxGetApp().WereRemoved(m_objects);
	wxGetApp().WasModified(m_owner);
	for(It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		object->m_owner = NULL;
	}

	m_belongs_to_owner = false;
}

const wxChar* AddObjectsTool::GetTitle()
{
	return _("Add Objects");
}

void AddObjectsTool::Run(bool redo)
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

void RemoveObjectsTool::Run(bool redo)
{
	Remove();
}

void RemoveObjectsTool::RollBack()
{
	Add();
}

CopyObjectUndoable::CopyObjectUndoable(HeeksObj* object, HeeksObj* copy_object): m_object(object), m_new_copy(copy_object)
{
	m_old_copy = m_object->MakeACopy();
}

CopyObjectUndoable::~CopyObjectUndoable()
{
	delete m_new_copy;
	delete m_old_copy;
}

void CopyObjectUndoable::Run(bool redo)
{
	m_object->CopyFrom(m_new_copy);
	wxGetApp().WasModified(m_object);
}

void CopyObjectUndoable::RollBack()
{
	m_object->CopyFrom(m_old_copy);
	wxGetApp().WasModified(m_object);
}
