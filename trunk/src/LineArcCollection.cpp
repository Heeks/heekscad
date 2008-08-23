// LineArcCollection.cpp

#include "stdafx.h"
#include "LineArcCollection.h"
#include "../interface/PropertyInt.h"
#include "../tinyxml/tinyxml.h"

wxIcon* CLineArcCollection::m_icon = NULL;
std::map<int, CLineArcCollection*> CLineArcCollection::used_ids;
int CLineArcCollection::next_id = 1;

void CLineArcCollection::set_initial()
{
	while(used_ids.find(next_id) != used_ids.end())next_id++;
	m_id = next_id;
	used_ids.insert( std::pair<int, CLineArcCollection*> (m_id, this) );
}

CLineArcCollection::CLineArcCollection()
{
	set_initial();
}

CLineArcCollection::CLineArcCollection(const CLineArcCollection& c)
{
	set_initial();
	operator=(c);
}

CLineArcCollection::~CLineArcCollection()
{
	next_id = m_id; // this id has now become available
	used_ids.erase(m_id);
}

const CLineArcCollection& CLineArcCollection::operator=(const CLineArcCollection& c)
{
	// just copy all the lines and arcs, not the id
	__super::operator =(c);

	return *this;
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
	CLineArcCollection::SetID(object_for_properties, value);
}

void CLineArcCollection::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);

	object_for_properties = this;
	list->push_back(new PropertyInt("ID", m_id, on_set_id));
	list->push_back(new PropertyInt("Number of elements", GetNumChildren()));
}

void CLineArcCollection::GetGripperPositions(std::list<double> *list, bool just_for_endof){
	// return the four corners of a 2d box
	CBox box;
	GetBox(box);
	list->push_back(0);
	list->push_back(box.m_x[0]);
	list->push_back(box.m_x[1]);
	list->push_back(box.m_x[2]);
	list->push_back(0);
	list->push_back(box.m_x[3]);
	list->push_back(box.m_x[1]);
	list->push_back(box.m_x[2]);
	list->push_back(0);
	list->push_back(box.m_x[3]);
	list->push_back(box.m_x[4]);
	list->push_back(box.m_x[2]);
	list->push_back(0);
	list->push_back(box.m_x[0]);
	list->push_back(box.m_x[4]);
	list->push_back(box.m_x[2]);
}

// static
HeeksObj* CLineArcCollection:: GetLineArcCollection(int id)
{
	std::map<int, CLineArcCollection*>::iterator FindIt = used_ids.find(id);
	if(FindIt == used_ids.end())return NULL;
	return FindIt->second;
}

// static
void CLineArcCollection::SetID(CLineArcCollection* la, int id)
{
	used_ids.erase(la->m_id);
	la->m_id = id;
	used_ids.insert( std::pair<int, CLineArcCollection*> (la->m_id, la) );
}

HeeksObj *CLineArcCollection::MakeACopy(void)const
{
	return new CLineArcCollection(*this);
}

void CLineArcCollection::WriteXML(TiXmlElement *root)
{
	TiXmlElement * element = new TiXmlElement( "LineArcCollection" );  
	root->LinkEndChild( element );
	element->SetAttribute("id", m_id);

	ObjList::WriteXML(element);
}

// static member function
HeeksObj* CLineArcCollection::ReadFromXMLElement(TiXmlElement* pElem)
{
	CLineArcCollection* new_object = new CLineArcCollection;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		wxString name(a->Name());
		if(name == "id"){CLineArcCollection::SetID(new_object, a->IntValue());}
	}

	new_object->ReadXMLChildren(pElem);

	return new_object;
}