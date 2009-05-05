// HText.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HText.h"
#include "../interface/PropertyDouble.h"
#include "PropertyTrsf.h"
#include "../tinyxml/tinyxml.h"
#include "Gripper.h"

HText::HText(const gp_Trsf &trsf, const wxString &text, const HeeksColor* col):m_color(*col),  m_trsf(trsf), m_text(text)
{
}

HText::HText(const HText &b)
{
	operator=(b);
}

HText::~HText(void)
{
}

const HText& HText::operator=(const HText &b)
{
	HeeksObj::operator=(b);
	m_trsf = b.m_trsf;
	m_text = b.m_text;
	m_color = b.m_color;
	return *this;
}

void HText::glCommands(bool select, bool marked, bool no_color)
{
	glPushMatrix();
	double m[16];
	extract_transposed(m_trsf, m);
	glMultMatrixd(m);

	if(!no_color)wxGetApp().glColorEnsuringContrast(m_color);

	wxGetApp().render_text(m_text);

	glPopMatrix();
}

void HText::GetBox(CBox &box)
{
	gp_Pnt vt(0, 0, 0);
	vt.Transform(m_trsf);
	double p[3];
	extract(vt, p);
	box.Insert(p);

	float width, height;
	if(!wxGetApp().get_text_size(m_text, &width, &height))return;

	gp_Pnt point[3];
	point[0] = gp_Pnt(width, 0, 0);
	point[1] = gp_Pnt(0, -height, 0);
	point[2] = gp_Pnt(width, -height, 0);

	for(int i = 0; i<3; i++)
	{
		point[i].Transform(m_trsf);
		extract(point[i], p);
		box.Insert(p);
	}
}

HeeksObj *HText::MakeACopy(void)const
{
	return new HText(*this);
}

bool HText::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_trsf = mat * m_trsf;
	return false;
}

void HText::GetGripperPositions(std::list<double> *list, bool just_for_endof)
{
	float width, height;
	if(!wxGetApp().get_text_size(m_text, &width, &height))return;

	gp_Pnt point[4];
	point[0] = gp_Pnt(0, 0, 0);
	point[1] = gp_Pnt(width, 0, 0);
	point[2] = gp_Pnt(0, -height, 0);
	point[3] = gp_Pnt(width, -height, 0);

	for(int i = 0; i<4; i++)point[i].Transform(m_trsf);

	list->push_back(GripperTypeTranslate);
	list->push_back(point[0].X());
	list->push_back(point[0].Y());
	list->push_back(point[0].Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(point[1].X());
	list->push_back(point[1].Y());
	list->push_back(point[1].Z());
	list->push_back(GripperTypeRotateObject);
	list->push_back(point[2].X());
	list->push_back(point[2].Y());
	list->push_back(point[2].Z());
	list->push_back(GripperTypeScale);
	list->push_back(point[3].X());
	list->push_back(point[3].Y());
	list->push_back(point[3].Z());
}

static void on_set_trsf(const gp_Trsf &trsf, HeeksObj* object){
	((HText*)object)->m_trsf = trsf;
	wxGetApp().Repaint();
}

void HText::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyTrsf(_("orientation"), m_trsf, this, on_set_trsf));

	HeeksObj::GetProperties(list);
}

bool HText::Stretch(const double *p, const double* shift)
{
	return false;
}

void HText::OnEditString(const wxChar* str){
	m_text.assign(str);
	wxGetApp().WasModified(this);
}

void HText::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "Text" );
	root->LinkEndChild( element );  
	element->SetAttribute("text", Ttc(m_text.c_str()));

	double m[16];
	extract(m_trsf, m);

	element->SetAttribute("col", m_color.COLORREF_color());
	element->SetDoubleAttribute("m0", m[0] );
	element->SetDoubleAttribute("m1", m[1] );
	element->SetDoubleAttribute("m2", m[2] );
	element->SetDoubleAttribute("m3", m[3] );
	element->SetDoubleAttribute("m4", m[4] );
	element->SetDoubleAttribute("m5", m[5] );
	element->SetDoubleAttribute("m6", m[6] );
	element->SetDoubleAttribute("m7", m[7] );
	element->SetDoubleAttribute("m8", m[8] );
	element->SetDoubleAttribute("m9", m[9] );
	element->SetDoubleAttribute("ma", m[10]);
	element->SetDoubleAttribute("mb", m[11]);

	WriteBaseXML(element);
}

// static
HeeksObj* HText::ReadFromXMLElement(TiXmlElement* pElem)
{
	double m[16];
	wxString text;
	HeeksColor c;

	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){c = HeeksColor(a->IntValue());}
		else if(name == "text"){text.assign(Ctt(a->Value()));}
		else if(name == "m0"){m[0] = a->DoubleValue();}
		else if(name == "m1"){m[1] = a->DoubleValue();}
		else if(name == "m2"){m[2] = a->DoubleValue();}
		else if(name == "m3"){m[3] = a->DoubleValue();}
		else if(name == "m4"){m[4] = a->DoubleValue();}
		else if(name == "m5"){m[5] = a->DoubleValue();}
		else if(name == "m6"){m[6] = a->DoubleValue();}
		else if(name == "m7"){m[7] = a->DoubleValue();}
		else if(name == "m8"){m[8] = a->DoubleValue();}
		else if(name == "m9"){m[9] = a->DoubleValue();}
		else if(name == "ma"){m[10]= a->DoubleValue();}
		else if(name == "mb"){m[11]= a->DoubleValue();}
	}

	HText* new_object = new HText(make_matrix(m), text, &c);
	new_object->ReadBaseXML(pElem);

	return new_object;
}
