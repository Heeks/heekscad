// LineArcCollection.cpp

#include "stdafx.h"
#include "LineArcCollection.h"
#include "../interface/PropertyInt.h"
#include "../tinyxml/tinyxml.h"

wxIcon* CLineArcCollection::m_icon = NULL;

CLineArcCollection::CLineArcCollection()
{
}

CLineArcCollection::CLineArcCollection(const CLineArcCollection& c)
{
	operator=(c);
}

CLineArcCollection::~CLineArcCollection()
{
}

const CLineArcCollection& CLineArcCollection::operator=(const CLineArcCollection& c)
{
	// just copy all the lines and arcs, not the id
	ObjList::operator =(c);

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

void CLineArcCollection::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);

	object_for_properties = this;
	list->push_back(new PropertyInt("Number of elements", ObjList::GetNumChildren()));
}

HeeksObj *CLineArcCollection::MakeACopy(void)const
{
	return (ObjList*)(new CLineArcCollection(*this));
}

void CLineArcCollection::WriteXML(TiXmlElement *root)
{
	TiXmlElement * element = new TiXmlElement( "LineArcCollection" );  
	root->LinkEndChild( element );
	WriteBaseXML(element);
}

// static member function
HeeksObj* CLineArcCollection::ReadFromXMLElement(TiXmlElement* pElem)
{
	CLineArcCollection* new_object = new CLineArcCollection;
	new_object->ReadBaseXML(pElem);

	return (ObjList*)new_object;
}