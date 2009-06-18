// ConstrainedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ConstrainedObject.h"

ConstrainedObject::ConstrainedObject(){
	absoluteanglecontraint = NULL;
}


ConstrainedObject::~ConstrainedObject(){
	if(absoluteanglecontraint)
		delete absoluteanglecontraint;
}

void ConstrainedObject::RemoveExisting(ConstrainedObject* obj)
{
	//Make sure there is not already a parallel or perpendicular constraint between
	//these objects
	std::list<Constraint>::iterator it;
	for(it = constraints.begin(); it!= constraints.end(); ++it)
	{
		Constraint c = *it;
		if(((ConstrainedObject*)c.m_obj1 == this && (ConstrainedObject*)c.m_obj2 == obj)||((ConstrainedObject*)c.m_obj1 == obj && (ConstrainedObject*)c.m_obj2 == this))
		{
			if(c.m_type == ParallelLineConstraint || c.m_type == PerpendicularLineConstraint)
			{
				constraints.remove(c);
				obj->constraints.remove(c);
			}
		}
	}
}

void ConstrainedObject::SetPerpendicularConstraint(ConstrainedObject* obj){
	RemoveExisting(obj);
	Constraint c;
	c.m_type = PerpendicularLineConstraint;
	c.m_obj1 = (HeeksObj*)this;
	c.m_obj2 = (HeeksObj*)obj;
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetParallelConstraint(ConstrainedObject* obj){
	RemoveExisting(obj);
	Constraint c;
	c.m_type = ParallelLineConstraint;
	c.m_obj1 = (HeeksObj*)this;
	c.m_obj2 = (HeeksObj*)obj;
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetAbsoluteAngleConstraint(EnumAbsoluteAngle angle)
{
	if(absoluteanglecontraint)
	{
		if(absoluteanglecontraint->m_angle == angle)
			delete absoluteanglecontraint;
		else
			absoluteanglecontraint->m_angle = angle;

	}
	else
		absoluteanglecontraint = new Constraint(AbsoluteAngleConstraint,angle,(HeeksObj*)this);
}

void ConstrainedObject::glCommands(HeeksColor color, gp_Ax1 mid_point)
{
	if(absoluteanglecontraint)
		absoluteanglecontraint->glCommands(color,mid_point);

	std::list<Constraint>::iterator it;
	for(it = constraints.begin(); it != constraints.end(); ++it)
	{
		Constraint c = *it;
		c.glCommands(color,mid_point);
	}
}
