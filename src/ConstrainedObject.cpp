// ConstrainedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ConstrainedObject.h"

ConstrainedObject::ConstrainedObject(){
	absoluteangleconstraint = NULL;
	linelengthconstraint = NULL;
	radiusconstraint = NULL;
}


ConstrainedObject::~ConstrainedObject(){
	if(absoluteangleconstraint)
		delete absoluteangleconstraint;
	if(linelengthconstraint)
		delete linelengthconstraint;
	if(radiusconstraint)
		delete radiusconstraint;
}

void ConstrainedObject::RemoveExisting(ConstrainedObject* obj, EnumConstraintType type)
{
	//Make sure there is not already a parallel or perpendicular constraint between
	//these objects
	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it!= constraints.end(); ++it)
	{
		Constraint *c = *it;
		if(((ConstrainedObject*)c->m_obj1 == this && (ConstrainedObject*)c->m_obj2 == obj)||((ConstrainedObject*)c->m_obj1 == obj && (ConstrainedObject*)c->m_obj2 == this))
		{
			if(c->m_type == type)
			{
				constraints.remove(c);
				obj->constraints.remove(c);
				delete c;
			}
		}
	}
}

void ConstrainedObject::SetPerpendicularConstraint(ConstrainedObject* obj){
	RemoveExisting(obj,PerpendicularLineConstraint);
	RemoveExisting(obj,ParallelLineConstraint);
	Constraint* c = new Constraint();
	c->m_type = PerpendicularLineConstraint;
	c->m_obj1 = (HeeksObj*)this;
	c->m_obj2 = (HeeksObj*)obj;
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetParallelConstraint(ConstrainedObject* obj){
	RemoveExisting(obj,PerpendicularLineConstraint);
	RemoveExisting(obj,ParallelLineConstraint);
	Constraint* c = new Constraint();
	c->m_type = ParallelLineConstraint;
	c->m_obj1 = (HeeksObj*)this;
	c->m_obj2 = (HeeksObj*)obj;
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetEqualLengthConstraint(ConstrainedObject* obj){
	RemoveExisting(obj, EqualLengthConstraint);
	Constraint* c = new Constraint();
	c->m_type = EqualLengthConstraint;
	c->m_obj1 = (HeeksObj*)this;
	c->m_obj2 = (HeeksObj*)obj;
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetColinearConstraint(ConstrainedObject* obj){
	RemoveExisting(obj,ColinearConstraint);
	Constraint* c = new Constraint();
	c->m_type = ColinearConstraint;
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

	if(linelengthconstraint)
		linelengthconstraint->glCommands(color,mid_point);

	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it != constraints.end(); ++it)
	{
		Constraint *c = *it;
		c->glCommands(color,mid_point);
	}
}

bool ConstrainedObject::HasConstraints()
{
	return absoluteangleconstraint || !constraints.empty() || linelengthconstraint || radiusconstraint;
}

bool ConstrainedObject::HasPointConstraint(ConstrainedObject* obj,EnumPoint obj1_point,EnumPoint obj2_point)
{
	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it!=constraints.end(); ++it)
	{
		Constraint* c = *it;
		if(c->m_type == CoincidantPointConstraint)
		{
			 if(c->m_obj1 == this && c->m_obj2 == obj)
				 if(c->m_obj1_point == obj1_point && c->m_obj2_point == obj2_point)
					 return true;
			 if(c->m_obj2 == this && c->m_obj1 == obj)
				 if(c->m_obj2_point == obj1_point && c->m_obj1_point == obj2_point)
					 return true;
		}
	}
	return false;
}

void ConstrainedObject::SetCoincidentPoint(ConstrainedObject* obj,EnumPoint obj1_point,EnumPoint obj2_point)
{
	//check for existing constraint
	if(HasPointConstraint(obj,obj1_point,obj2_point))
		return;

	//add new constraint
	Constraint *c = new Constraint(CoincidantPointConstraint,obj1_point,obj2_point,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetLineLengthConstraint(double length)
{
	if(linelengthconstraint)
	{
		delete linelengthconstraint;
		linelengthconstraint = NULL;
	}
	else
		linelengthconstraint = new Constraint(LineLengthConstraint,length,(HeeksObj*)this);
}

void ConstrainedObject::SetRadiusConstraint(double length)
{
	if(radiusconstraint)
	{
		delete radiusconstraint;
		radiusconstraint = NULL;
	}
	else
		radiusconstraint = new Constraint(RadiusConstraint,length,(HeeksObj*)this);
}


void ConstrainedObject::SetTangentConstraint(ConstrainedObject* obj)
{
	//Check for existing tangent constraint to the same object
	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it!=constraints.end(); ++it)
	{
		Constraint* c = *it;
		if(c->m_type == LineTangentToArcConstraint && (ConstrainedObject*)c->m_obj2 == obj)
		{
			constraints.remove(c);
			delete c;
			return;
		}
	}
	
	Constraint *c = new Constraint(LineTangentToArcConstraint,this,obj);
	constraints.push_back(c);
}

void ConstrainedObject::SetLineLength(double length)
{
	if(linelengthconstraint)
	{
		linelengthconstraint->m_length = length;
	}
}

void ConstrainedObject::SetRadius(double radius)
{
	if(radiusconstraint)
	{
		radiusconstraint->m_length = radius;
	}
}