// ConstrainedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "Constraint.h"
#include "ConstrainedObject.h"
#include "../tinyxml/tinyxml.h"

std::string AbsoluteAngle[] = {
	"AbsoluteAngleHorizontal",
	"AbsoluteAngleVertical"
};

std::string ConstraintTypes[]={
	"CoincidantPointConstraint",
	"ParallelLineConstraint",
	"PerpendicularLineConstraint",
	"AbsoluteAngleConstraint",
	"LineLengthConstraint",
	"LineTangentConstraint",
	"RadiusConstraint",
	"EqualLengthConstraint",
	"ColinearConstraint",
	"EqualRadiusConstraint",
	"ConcentricConstraint",
	"PointOnLineConstraint",
	"PointOnLineMidpointConstraint",
	"PointOnArcMidpointConstraint",
	"PointOnArcConstraint"
};


Constraint::Constraint(){
	
}

Constraint::Constraint(const Constraint* obj){
	m_type = obj->m_type;
	m_angle = obj->m_angle;
	m_length = obj->m_length;
	m_obj1 = obj->m_obj1;
	m_obj2 = obj->m_obj2;
}

Constraint::Constraint(EnumConstraintType type,EnumAbsoluteAngle angle, ConstrainedObject* obj)
{
    m_type = type;
	m_angle = angle;
	m_obj1 = obj;
	m_obj2 = NULL;
	m_length = 0;
	m_obj1->Add(this,NULL);
}

Constraint::Constraint(EnumConstraintType type,ConstrainedObject* obj1,ConstrainedObject* obj2)
{
    m_type = type;
	m_angle = (EnumAbsoluteAngle)0;
	m_obj1 = obj1;
	m_obj2 = obj2;
	m_length = 0;
	m_obj1->Add(this,NULL);
	m_obj2->Add(this,NULL);
}


Constraint::Constraint(EnumConstraintType type,double length,ConstrainedObject* obj1)
{
    m_type = type;
	m_angle = (EnumAbsoluteAngle)0;
	m_obj1 = obj1;
	m_obj2 = 0;
	m_length = length;
}

const Constraint& Constraint::operator=(const Constraint &b){
	HeeksObj::operator=(b);
	m_type = b.m_type;
	m_angle = b.m_angle;
	m_obj1 = b.m_obj1;
	m_obj2 = b.m_obj2;
	m_length = b.m_length;
	return *this;
}

Constraint::~Constraint()
{
	if(m_obj1)
		m_obj1->Remove(this);
	if(m_obj2)
		m_obj2->Remove(this);
}

void Constraint::render_text(const wxChar* str)
{
	wxGetApp().create_font();
	//glColor4ub(0, 0, 0, 255);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable(GL_TEXTURE_2D);
	glDepthMask(0);
	glDisable(GL_POLYGON_OFFSET_FILL);
	wxGetApp().m_gl_font.Begin();

	//Draws text with a glFont
	float scale = 0.08f;
	std::pair<int,int> size;
	wxGetApp().m_gl_font.GetStringSize(str,&size);
	wxGetApp().m_gl_font.DrawString(str, scale, -size.first/2.0f*scale, size.second/2.0f*scale);

	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void Constraint::glCommands(HeeksColor color, gp_Ax1 axis)
{
	double mag = sqrt(axis.Direction().X() * axis.Direction().X() + axis.Direction().Y() * axis.Direction().Y());
	float rot = (float)atan2(axis.Direction().Y()/mag,axis.Direction().X()/mag);

	glPushMatrix();
	glTranslatef((float)axis.Location().X(),(float)axis.Location().Y(),(float)axis.Location().Z());
	glRotatef(rot*180/(float)Pi,0,0,1);
	glTranslatef(0,ANGLE_OFFSET_FROM_LINE,0);

	switch(m_type)
	{
		case AbsoluteAngleConstraint:
			switch(m_angle)
			{
				case AbsoluteAngleHorizontal:
					render_text(_("H"));
					break;
				case AbsoluteAngleVertical:
					glRotatef(90,0,0,1);
					render_text(_("V"));
					break;
			}
			break;
		case LineLengthConstraint:
			wxChar str[100];
			wxSprintf(str,_("%f"),m_length);
			render_text(str);
			break;
		case ParallelLineConstraint:
			render_text(_("L"));
			break;
		case PerpendicularLineConstraint:
			render_text(_("P"));
			break;
		default:
			break;
	}

	glPopMatrix();
}

HeeksObj *Constraint::MakeACopy(void)const
{
	return new Constraint(this);
}

void Constraint::WriteXML(TiXmlNode *root)
{
	return; //Disabled for now. These should not be children of the lines/points
	TiXmlElement * element;
	element = new TiXmlElement( "Constraint" );
	root->LinkEndChild( element );  
	element->SetAttribute("type", ConstraintTypes[m_type].c_str());
	element->SetAttribute("angle", AbsoluteAngle[m_angle].c_str());
	element->SetDoubleAttribute("length", m_length);
	element->SetAttribute("obj1_id",m_obj1->m_id);
	element->SetAttribute("obj1_type",m_obj1->GetType());
	if(m_obj2)
	{
		element->SetAttribute("obj2_id",m_obj2->m_id);
		element->SetAttribute("obj2_type",m_obj2->GetType());
	}

	WriteBaseXML(element);
}

HeeksObj* Constraint::ReadFromXMLElement(TiXmlElement* pElem)
{
	return NULL;
}