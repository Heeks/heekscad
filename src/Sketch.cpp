// Sketch.cpp

#include "stdafx.h"
#include "Sketch.h"
#include "../interface/PropertyInt.h"
#include "../tinyxml/tinyxml.h"

wxIcon* CSketch::m_icon = NULL;

CSketch::CSketch()
{
}

CSketch::CSketch(const CSketch& c)
{
	operator=(c);
}

CSketch::~CSketch()
{
}

const CSketch& CSketch::operator=(const CSketch& c)
{
	// just copy all the lines and arcs, not the id
	ObjList::operator =(c);

	return *this;
}

wxIcon* CSketch::GetIcon(){
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + _T("/icons/linedrawing.png"), wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}

void CSketch::GetProperties(std::list<Property *> *list)
{
	__super::GetProperties(list);

	list->push_back(new PropertyInt(_T("Number of elements"), ObjList::GetNumChildren(), this));
}

HeeksObj *CSketch::MakeACopy(void)const
{
	return (ObjList*)(new CSketch(*this));
}

void CSketch::WriteXML(TiXmlElement *root)
{
	TiXmlElement * element = new TiXmlElement( "Sketch" );  
	root->LinkEndChild( element );
	WriteBaseXML(element);
}

// static member function
HeeksObj* CSketch::ReadFromXMLElement(TiXmlElement* pElem)
{
	CSketch* new_object = new CSketch;
	new_object->ReadBaseXML(pElem);

	return (ObjList*)new_object;
}