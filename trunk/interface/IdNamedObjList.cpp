// IdNamedObjList.cpp

#include <stdafx.h>

#include "IdNamedObjList.h"

static wxString temp_pattern_string;

void IdNamedObjList::WriteBaseXML(TiXmlElement *element)
{
	element->SetAttribute("title_from_id", m_title_made_from_id ? 1:0);
	if(!m_title_made_from_id)element->SetAttribute("title", Ttc(m_title.c_str()));
	ObjList::WriteBaseXML(element);
}

void IdNamedObjList::ReadBaseXML(TiXmlElement* element)
{
	if(const char* pstr = element->Attribute("title"))m_title = Ctt(pstr);
	int i;
	if(element->Attribute("title_from_id", &i))m_title_made_from_id = (i != 0);
	ObjList::ReadBaseXML(element);
}

const wxChar* IdNamedObjList::GetShortString(void)const
{
	if(m_title_made_from_id)
	{
		temp_pattern_string = wxString::Format(_T("%s %d"), GetTypeString(), m_id);
		return temp_pattern_string;
	}
	return m_title.c_str();}

void IdNamedObjList::OnEditString(const wxChar* str)
{
    m_title.assign(str);
	m_title_made_from_id = false;
}
