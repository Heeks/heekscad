// ConstrainedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "Constraint.h"

Constraint::Constraint(){
	
}

Constraint::Constraint(EnumConstraintType type,EnumAbsoluteAngle angle, HeeksObj* obj)
{
    m_type = type;
	m_angle = angle;
	m_obj1 = obj;
	m_obj2 = NULL;
	m_obj1_point = PointA;
	m_obj2_point = PointA;
	m_length = 0;
}

Constraint::Constraint(EnumConstraintType type,HeeksObj* obj1,HeeksObj* obj2)
{
    m_type = type;
	m_angle = (EnumAbsoluteAngle)0;
	m_obj1 = obj1;
	m_obj2 = obj2;
	m_obj1_point = PointA;
	m_obj2_point = PointA;
	m_length = 0;
}


Constraint::Constraint(EnumConstraintType type,EnumPoint point1,EnumPoint point2,HeeksObj* obj1, HeeksObj* obj2)
{
    m_type = type;
	m_angle = (EnumAbsoluteAngle)0;
	m_obj1 = obj1;
	m_obj2 = obj2;
	m_obj1_point = point1;
	m_obj2_point = point2;
	m_length = 0;
}

Constraint::Constraint(EnumConstraintType type,double length,HeeksObj* obj1)
{
    m_type = type;
	m_angle = (EnumAbsoluteAngle)0;
	m_obj1 = obj1;
	m_obj2 = 0;
	m_obj1_point = (EnumPoint)0;
	m_obj2_point = (EnumPoint)0;
	m_length = length;
}


Constraint::~Constraint(){
}

void Constraint::render_text(const wxChar* str)
{
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
