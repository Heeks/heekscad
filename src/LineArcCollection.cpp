// LineArcCollection.cpp

#include "stdafx.h"
#include "LineArcCollection.h"
#include "../interface/PropertyInt.h"

wxIcon* CLineArcCollection::m_icon = NULL;
std::map<int, CLineArcCollection*> CLineArcCollection::used_ids;
int CLineArcCollection::next_id = 1;

CLineArcCollection::CLineArcCollection()
{
	while(used_ids.find(next_id) != used_ids.end())next_id++;
	m_id = next_id;
	used_ids.insert( std::pair<int, CLineArcCollection*> (m_id, this) );
}

CLineArcCollection::~CLineArcCollection()
{
	next_id = m_id; // this id has now become available
	used_ids.erase(m_id);
}

wxIcon* CLineArcCollection::GetIcon(){
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + "/icons/linedrawing.png", wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}

static CLineArcCollection* object_for_properties = NULL;

static void on_set_id(int value)
{
	CLineArcCollection::used_ids.erase(object_for_properties->m_id);
	object_for_properties->m_id = value;
	CLineArcCollection::used_ids.insert( std::pair<int, CLineArcCollection*> (object_for_properties->m_id, object_for_properties) );
}

void CLineArcCollection::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);

	object_for_properties = this;
	list->push_back(new PropertyInt("ID", m_id, on_set_id));
}

// static
HeeksObj* CLineArcCollection:: GetLineArcCollection(int id)
{
	std::map<int, CLineArcCollection*>::iterator FindIt = used_ids.find(id);
	if(FindIt == used_ids.end())return NULL;
	return FindIt->second;
}