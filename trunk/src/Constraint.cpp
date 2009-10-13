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
	m_obj1->Add(this,NULL);
	if(m_obj2)
		m_obj2->Add(this,NULL);

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

Constraint::Constraint(EnumConstraintType type,EnumAbsoluteAngle angle, double length, ConstrainedObject* obj1, ConstrainedObject* obj2)
{
    m_type = type;
	m_angle = angle;
	m_obj1 = obj1;
	m_obj2 = obj2;
	m_length = length;
	m_obj1->Add(this,NULL);
	if(m_obj2)
		m_obj2->Add(this,NULL);
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
	obj1->Add(this,NULL);
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

}

void Constraint::Disconnect(std::list<HeeksObj*> parents)
{
	HeeksObj* owner = GetFirstOwner();
	if(parents.back() == owner)
	{
		this->RemoveOwner(owner);
		return;
	}
	owner = GetNextOwner();
	if(parents.back() == owner)
		RemoveOwner(owner);
}

void Constraint::ReloadPointers()
{
	m_obj1 = (ConstrainedObject*)GetFirstOwner();
	m_obj2 = (ConstrainedObject*)GetNextOwner();
	if(!m_obj2)
	{
		int x=0;
		x++;
	}
}

bool Constraint::IsDifferent(HeeksObj* o)
{
	Constraint* other = (Constraint*)o;
	if(m_type != other->m_type || m_angle != other->m_angle || m_length != other->m_length)
		return true;

	int id1_1=0;
	int id1_2=0;
	int id2_1=0;
	int id2_2=0;

	if(m_obj1)
		id1_1 = m_obj1->m_id;
	if(other->m_obj1)
		id1_2 = other->m_obj1->m_id;
	if(m_obj2)
		id2_1 = m_obj2->m_id;
	if(other->m_obj2)
		id2_2 = other->m_obj2->m_id;

	if(id1_1 != id1_2 && id1_1 != id2_2)
		return true;

	if(id2_1 != id2_2 && id2_1 != id1_2)
		return true;

	return false;
}

void Constraint::render_text(const wxChar* str)
{
	wxGetApp().create_font();
	//glColor4ub(0, 0, 0, 255);
	wxGetApp().EnableBlend();
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
	wxGetApp().DisableBlend();
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
	return new Constraint(*this);
}

static std::list<Constraint*> obj_to_save;
static std::set<Constraint*> obj_to_save_find;

void Constraint::BeginSave()
{
	obj_to_save.clear();
	obj_to_save_find.clear();
}

void Constraint::EndSave(TiXmlNode *root)
{
	std::list<Constraint*>::iterator it;
	for(it = obj_to_save.begin(); it != obj_to_save.end(); it++)
	{
		Constraint *c = *it;
		TiXmlElement * element;
		element = new TiXmlElement( "Constraint" );	
		root->LinkEndChild( element );  
		element->SetAttribute("type", ConstraintTypes[c->m_type].c_str());
		element->SetAttribute("angle", AbsoluteAngle[c->m_angle].c_str());
		element->SetDoubleAttribute("length", c->m_length);
		element->SetAttribute("obj1_id",c->m_obj1->m_id);
		element->SetAttribute("obj1_type",c->m_obj1->GetIDGroupType());
		if(c->m_obj2)
		{
			element->SetAttribute("obj2_id",c->m_obj2->m_id);
			element->SetAttribute("obj2_type",c->m_obj2->GetIDGroupType());
		}
		c->WriteBaseXML(element);
	}
}

void Constraint::WriteXML(TiXmlNode *root)
{
	if(obj_to_save_find.find(this)!=obj_to_save_find.end())
		return;

	obj_to_save.push_back(this);
	obj_to_save_find.insert(this);
}

HeeksObj* Constraint::ReadFromXMLElement(TiXmlElement* pElem)
{
	const char* type=0;
	EnumConstraintType etype=(EnumConstraintType)0;
	const char* angle=0;
	EnumAbsoluteAngle eangle=(EnumAbsoluteAngle)0;
	double length=0;
	int obj1_id=0;
	int obj2_id=0;
	int obj1_type=0;
	int obj2_type=0;
	ConstrainedObject* obj1=0;
	ConstrainedObject* obj2=0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "type"){type = a->Value();}
		else if(name == "angle"){angle = a->Value();}
		else if(name == "length"){length = a->DoubleValue();}
		else if(name == "obj1_id"){obj1_id = a->IntValue();}
		else if(name == "obj1_type"){obj1_type = a->IntValue();}
		else if(name == "obj2_id"){obj2_id = a->IntValue();}
		else if(name == "obj2_type"){obj2_type = a->IntValue();}
	}

	//Ugh, we need to convert the strings back into types
	for(unsigned i=0; i < sizeof(ConstraintTypes); i++)
	{
		if(strcmp(ConstraintTypes[i].c_str(),type)==0)
		{
			etype = (EnumConstraintType)i;
			break;
		}
	}

	for(unsigned i=0; i < sizeof(AbsoluteAngle); i++)
	{
		if(strcmp(AbsoluteAngle[i].c_str(),angle)==0)
		{
			eangle = (EnumAbsoluteAngle)i;
			break;
		}
	}

	//Get real pointers to the objects
	obj1 = (ConstrainedObject*)wxGetApp().GetIDObject(obj1_type,obj1_id);
	obj2 = (ConstrainedObject*)wxGetApp().GetIDObject(obj2_type,obj2_id);

	if(obj1 == NULL || obj2 == NULL)return NULL;

	Constraint *c = new Constraint(etype,eangle,length,obj1,obj2);

	obj1->constraints.push_back(c);
	obj2->constraints.push_back(c);

	//Don't let the xml reader try to insert us in the tree
	return NULL;
}
