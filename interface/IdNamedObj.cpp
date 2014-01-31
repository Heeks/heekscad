// IdNamedObj.cpp

#include <stdafx.h>

#include "IdNamedObj.h"

static wxString temp_pattern_string;

void IdNamedObj::WriteBaseXML(TiXmlElement *element)
{
	element->SetAttribute("title_from_id", m_title_made_from_id ? 1:0);
	if(!m_title_made_from_id)element->SetAttribute("title", Ttc(m_title.c_str()));
	HeeksObj::WriteBaseXML(element);
}

void IdNamedObj::ReadBaseXML(TiXmlElement* element)
{
	if(const char* pstr = element->Attribute("title"))m_title = Ctt(pstr);
	int i;
	if(element->Attribute("title_from_id", &i))m_title_made_from_id = (i != 0);
	HeeksObj::ReadBaseXML(element);
}

const wxChar* IdNamedObj::GetShortString(void)const
{
	if(m_title_made_from_id)
	{
		wxChar pattern_str[512];
		wsprintf(pattern_str, _T("%s %d"), GetTypeString(), m_id);
		temp_pattern_string.assign(pattern_str);
		return temp_pattern_string;
	}
	return m_title.c_str();}

void IdNamedObj::OnEditString(const wxChar* str)
{
    m_title.assign(str);
	m_title_made_from_id = false;
}
