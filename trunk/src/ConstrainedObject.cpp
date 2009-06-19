// ConstrainedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ConstrainedObject.h"

ConstrainedObject::ConstrainedObject(){
	absoluteangleconstraint = NULL;
}


ConstrainedObject::~ConstrainedObject(){
	if(absoluteangleconstraint)
		delete absoluteangleconstraint;
}

void ConstrainedObject::RemoveExisting(ConstrainedObject* obj)
{
	//Make sure there is not already a parallel or perpendicular constraint between
	//these objects
	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it!= constraints.end(); ++it)
	{
		Constraint *c = *it;
		if(((ConstrainedObject*)c->m_obj1 == this && (ConstrainedObject*)c->m_obj2 == obj)||((ConstrainedObject*)c->m_obj1 == obj && (ConstrainedObject*)c->m_obj2 == this))
		{
			if(c->m_type == ParallelLineConstraint || c->m_type == PerpendicularLineConstraint)
			{
				constraints.remove(c);
				obj->constraints.remove(c);
				delete c;
			}
		}
	}
}

void ConstrainedObject::SetPerpendicularConstraint(ConstrainedObject* obj){
	RemoveExisting(obj);
	Constraint* c = new Constraint();
	c->m_type = PerpendicularLineConstraint;
	c->m_obj1 = (HeeksObj*)this;
	c->m_obj2 = (HeeksObj*)obj;
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetParallelConstraint(ConstrainedObject* obj){
	RemoveExisting(obj);
	Constraint* c = new Constraint();
	c->m_type = ParallelLineConstraint;
	c->m_obj1 = (HeeksObj*)this;
	c->m_obj2 = (HeeksObj*)obj;
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetAbsoluteAngleConstraint(EnumAbsoluteAngle angle)
{
	if(absoluteangleconstraint)
	{
		if(absoluteangleconstraint->m_angle == angle)
		{
			delete absoluteangleconstraint;
			absoluteangleconstraint=NULL;
		}
		else
			absoluteangleconstraint->m_angle = angle;

	}
	else
		absoluteangleconstraint = new Constraint(AbsoluteAngleConstraint,angle,(HeeksObj*)this);
}

void ConstrainedObject::glCommands(HeeksColor color, gp_Ax1 mid_point)
{
	if(absoluteangleconstraint)
		absoluteangleconstraint->glCommands(color,mid_point);

	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it != constraints.end(); ++it)
	{
		Constraint *c = *it;
		c->glCommands(color,mid_point);
	}
}

bool ConstrainedObject::HasConstraints()
{
	return absoluteangleconstraint || !constraints.empty();
}