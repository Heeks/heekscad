// ObjList.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "ObjList.h"
#ifdef HEEKSCAD
#include "../src/MarkedList.h"
#else
#include "HeeksCADInterface.h"
#endif
#include "../tinyxml/tinyxml.h"


ObjList::ObjList(const ObjList& objlist): HeeksObj(objlist), m_index_list_valid(true) {operator=(objlist);}

void ObjList::Clear()
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		(*It)->RemoveOwners();
		delete *It;
	}
	m_objects.clear();
	m_index_list.clear();
	m_index_list_valid = true;
}

void ObjList::Clear(std::set<HeeksObj*> &to_delete)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end();)
	{
		if(to_delete.find(*It) != to_delete.end())
		{
			(*It)->RemoveOwners();
			It = m_objects.erase(It);
		}
		else
			It++;
	}
	m_index_list.clear();
	m_index_list_valid = false;
}

const ObjList& ObjList::operator=(const ObjList& objlist)
{
	HeeksObj::operator=(objlist);
	Clear();
	std::list<HeeksObj*>::const_iterator It;
	for (It=objlist.m_objects.begin();It!=objlist.m_objects.end();It++)
	{
		HeeksObj* new_op = (*It)->MakeACopy();
		if(new_op)Add(new_op, NULL);
	}
	return *this;
}

void ObjList::ClearUndoably(void)
{
	if (m_objects.size() == 0) return;
	std::list<HeeksObj*> objects_to_delete = m_objects;
	std::list<HeeksObj*>::iterator It;
	for (It=objects_to_delete.begin();It!=objects_to_delete.end();It++)
	{
#ifdef HEEKSCAD
		wxGetApp().DeleteUndoably(*It);
#else
		heeksCAD->DeleteUndoably(*It);
#endif
	}
	m_objects.clear();
	LoopItStack.clear();
	m_index_list.clear();
	m_index_list_valid = true;
}

HeeksObj* ObjList::MakeACopy(void) const { return new ObjList(*this); }

void ObjList::GetBox(CBox &box)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(object->OnVisibleLayer() && object->m_visible)
		{
			object->GetBox(box);
		}
	}
}

void ObjList::glCommands(bool select, bool marked, bool no_color)
{
	HeeksObj::glCommands(select, marked, no_color);
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(object->OnVisibleLayer() && object->m_visible)
		{
			if(select)glPushName((unsigned long)object);
#ifdef HEEKSCAD
			(*It)->glCommands(select, marked || wxGetApp().m_marked_list->ObjectMarked(object), no_color);
#else
			(*It)->glCommands(select, marked || heeksCAD->ObjectMarked(object), no_color);
#endif
			if(select)glPopName();
		}
	}
}

void ObjList::Draw(wxDC& dc){
	HeeksObj::Draw(dc);
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(object->OnVisibleLayer() && object->m_visible)
		{
			object->Draw(dc);
		}
	}
}

HeeksObj* ObjList::GetFirstChild()
{
	if (m_objects.size()==0) return NULL;
	LoopIt = m_objects.begin();
	return *LoopIt;
}

HeeksObj* ObjList::GetNextChild()
{
	if (m_objects.size()==0 || LoopIt==m_objects.end()) return NULL;
	LoopIt++;
	if (LoopIt==m_objects.end()) return NULL;
	return *LoopIt;
}

void ObjList::recalculate_index_list()
{
	m_index_list.clear();
	m_index_list.resize(m_objects.size());
	int i = 0;
	for(std::list<HeeksObj*>::iterator It=m_objects.begin(); It!=m_objects.end() ;It++, i++)
	{
		HeeksObj* object = *It;
		m_index_list[i] = object;
	}
	m_index_list_valid = true;
}

HeeksObj* ObjList::GetAtIndex(int index)
{
	if(!m_index_list_valid)
	{
		recalculate_index_list();
	}

	if(index < 0 || index >= (int)(m_index_list.size()))return NULL;
	return m_index_list[index];
}

int ObjList::GetNumChildren()
{
	return m_objects.size();
}

bool ObjList::Add(HeeksObj* object, HeeksObj* prev_object)
{
	if (object==NULL) return false;
	if (!CanAdd(object)) return false;
	if (m_objects.size()==0 || prev_object==NULL)
	{
		m_objects.push_back(object);
		LoopIt = m_objects.end();
		LoopIt--;
	}
	else
	{
		for(LoopIt = m_objects.begin(); LoopIt != m_objects.end(); LoopIt++) { if (*LoopIt==prev_object) break; }
		m_objects.insert(LoopIt, object);
	}
	m_index_list_valid = false;
	HeeksObj::Add(object, prev_object);

#ifdef HEEKSCAD
	if((!wxGetApp().m_in_OpenFile || wxGetApp().m_file_open_or_import_type != FileOpenTypeHeeks) && object->UsesID() && (object->m_id == 0 || (wxGetApp().m_file_open_or_import_type == FileImportTypeHeeks && wxGetApp().m_in_OpenFile)))
	{
		object->SetID(wxGetApp().GetNextID(object->GetIDGroupType()));
	}
#else
	if(!heeksCAD->InOpenFile() && object->UsesID() && object->m_id == 0)
	{
		object->SetID(heeksCAD->GetNextID(object->GetIDGroupType()));
	}
#endif

	return true;
}


void ObjList::Remove(HeeksObj* object)
{
	if (object==NULL) return;
	for(LoopIt = m_objects.begin(); LoopIt != m_objects.end(); LoopIt++){
		if(*LoopIt==object)break;
	}
	if(LoopIt != m_objects.end())
	{
		m_objects.erase(LoopIt);
	}
	m_index_list_valid = false;
	HeeksObj::Remove(object);

#ifdef HEEKSCAD
	if((!wxGetApp().m_in_OpenFile || wxGetApp().m_file_open_or_import_type != FileOpenTypeHeeks) && object->UsesID() && (object->m_id == 0 || (wxGetApp().m_file_open_or_import_type == FileImportTypeHeeks && wxGetApp().m_in_OpenFile)))
	{
		wxGetApp().RemoveID(object);
	}
#else
	if(!heeksCAD->InOpenFile() && object->UsesID() && object->m_id == 0)
	{
		heeksCAD->RemoveID(object);
	}
#endif
}

void ObjList::KillGLLists(void)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++) (*It)->KillGLLists();
}

void ObjList::WriteBaseXML(TiXmlElement *element)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++) (*It)->WriteXML(element);
	HeeksObj::WriteBaseXML(element);
}

void ObjList::ReadBaseXML(TiXmlElement* element)
{
	// loop through all the objects
	for(TiXmlElement* pElem = TiXmlHandle(element).FirstChildElement().Element(); pElem;	pElem = pElem->NextSiblingElement())
	{
#ifdef HEEKSCAD
		HeeksObj* object = wxGetApp().ReadXMLElement(pElem);
#else
		HeeksObj* object = heeksCAD->ReadXMLElement(pElem);
#endif
		if(object)Add(object, NULL);
	}

	HeeksObj::ReadBaseXML(element);
}

bool ObjList::ModifyByMatrix(const double *m)
{
	bool done_with_add_and_remove = false;
	std::list<HeeksObj*> copy_list = m_objects;
	for(std::list<HeeksObj*>::iterator It=copy_list.begin(); It!=copy_list.end() ;It++)
	{
		if((*It)->ModifyByMatrix(m))done_with_add_and_remove = true;
	}

	return done_with_add_and_remove;
}

void ObjList::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal)
{
	for(std::list<HeeksObj*>::iterator It=m_objects.begin(); It!=m_objects.end() ;It++) (*It)->GetTriangles(callbackfunc, cusp, just_one_average_normal);
}

