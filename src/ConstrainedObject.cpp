// ConstrainedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ConstrainedObject.h"
#include "HPoint.h"

ConstrainedObject::ConstrainedObject(){
}


ConstrainedObject::~ConstrainedObject(){
}

const ConstrainedObject& ConstrainedObject::operator=(const ConstrainedObject &b){
	ObjList::operator=(b);
	return *this;
}

void ConstrainedObject::ReloadPointers()
{
	std::list<HeeksObj*>::iterator it;
	constraints.clear();

	for(it = m_objects.begin(); it!= m_objects.end(); it++)
	{
		Constraint* c = dynamic_cast<Constraint*>(*it);
		if(c)
			constraints.push_back(c);
	}
}

void ConstrainedObject::LoadToDoubles()
{

}

void ConstrainedObject::LoadFromDoubles()
{

}

Constraint* ConstrainedObject::GetExisting(EnumConstraintType type)
{
	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it!= constraints.end(); ++it)
	{
		Constraint *c = *it;
		if(c->m_type == type)
		{
			return c;
		}
	}
	return NULL;
}

bool ConstrainedObject::RemoveExisting(HeeksObj* obj, EnumConstraintType type)
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
				ConstrainedObject* cobj = dynamic_cast<ConstrainedObject*>(obj);
				if(cobj)
					cobj->constraints.remove(c);
				delete c;
				return true;
			}
		}
	}
	return false;
}

bool ConstrainedObject::SetPerpendicularConstraint(ConstrainedObject* obj){
	if(RemoveExisting(obj,PerpendicularLineConstraint)) return true;
	if(RemoveExisting(obj,ParallelLineConstraint)) return true;
	Constraint* c = new Constraint(PerpendicularLineConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
	return false;
}

bool ConstrainedObject::SetParallelConstraint(ConstrainedObject* obj){
	if(RemoveExisting(obj,PerpendicularLineConstraint)) return true;
	if(RemoveExisting(obj,ParallelLineConstraint)) return true;
	Constraint* c = new Constraint(ParallelLineConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
	return false;
}

bool ConstrainedObject::SetEqualLengthConstraint(ConstrainedObject* obj){
	if(RemoveExisting(obj, EqualLengthConstraint)) return true;
	Constraint* c = new Constraint(EqualLengthConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
	return false;
}

bool ConstrainedObject::SetConcentricConstraint(ConstrainedObject* obj){
	if(RemoveExisting(obj, ConcentricConstraint)) return true;
	Constraint* c = new Constraint(ConcentricConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
	return false;
}


bool ConstrainedObject::SetEqualRadiusConstraint(ConstrainedObject* obj){
	if(RemoveExisting(obj, EqualRadiusConstraint)) return true;
	Constraint* c = new Constraint(EqualRadiusConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
	return false;
}


bool ConstrainedObject::SetColinearConstraint(ConstrainedObject* obj){
	if(RemoveExisting(obj,ColinearConstraint)) return true;
	Constraint* c = new Constraint(ColinearConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
	return false;
}

void ConstrainedObject::SetAbsoluteAngleConstraint(EnumAbsoluteAngle angle)
{
	Constraint* c = GetExisting(AbsoluteAngleConstraint);
	if(c)
	{
		if(c->m_angle == angle)
		{
			constraints.remove(c);
			delete c;
		}
		else
			c->m_angle = angle;

	}
	else
		constraints.push_back(new Constraint(AbsoluteAngleConstraint,angle,this));
}

void ConstrainedObject::glCommands(HeeksColor color, gp_Ax1 mid_point)
{
	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it != constraints.end(); ++it)
	{
		Constraint *c = *it;
		c->glCommands(color,mid_point);
	}
}

bool ConstrainedObject::HasConstraints()
{
	return !constraints.empty();
}

bool ConstrainedObject::HasPointConstraint(ConstrainedObject* obj)
{
	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it!=constraints.end(); ++it)
	{
		Constraint* c = *it;
		if(c->m_type == CoincidantPointConstraint)
		{
			 if(c->m_obj1 == this && c->m_obj2 == obj)
					 return true;
			 if(c->m_obj2 == this && c->m_obj1 == obj)
					 return true;
		}
	}
	return false;
}

void ConstrainedObject::SetCoincidentPoint(ConstrainedObject* obj, bool remove)
{
	//check for existing constraint
	if(!remove && HasPointConstraint(obj))
		return;

	if(remove && RemoveExisting(obj,CoincidantPointConstraint))
		return;



	//add new constraint
	Constraint *c = new Constraint(CoincidantPointConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetLineLengthConstraint(double length)
{
	Constraint *c = GetExisting(LineLengthConstraint);
	if(c)
	{
		constraints.remove(c);
	}
	else
		constraints.push_back(new Constraint(LineLengthConstraint,length,this));
}

void ConstrainedObject::SetLineHorizontalLengthConstraint(double length)
{
	Constraint *c = GetExisting(LineHorizontalLengthConstraint);
	if(c)
	{
		constraints.remove(c);
	}
	else
		constraints.push_back(new Constraint(LineHorizontalLengthConstraint,length,this));
}

void ConstrainedObject::SetLineVerticalLengthConstraint(double length)
{
	Constraint *c = GetExisting(LineVerticalLengthConstraint);
	if(c)
	{
		constraints.remove(c);
	}
	else
		constraints.push_back(new Constraint(LineVerticalLengthConstraint,length,this));
}

void ConstrainedObject::SetRadiusConstraint(double length)
{
	Constraint *c = GetExisting(RadiusConstraint);
	if(c)
	{
		constraints.remove(c);
	}
	else
		constraints.push_back(new Constraint(RadiusConstraint,length,this));
}


void ConstrainedObject::SetTangentConstraint(ConstrainedObject* obj)
{
	//Check for existing tangent constraint to the same object
	std::list<Constraint*>::iterator it;
	for(it = constraints.begin(); it!=constraints.end(); ++it)
	{
		Constraint* c = *it;
		if(c->m_type == LineTangentConstraint && (ConstrainedObject*)c->m_obj2 == obj)
		{
			constraints.remove(c);
			return;
		}
	}
	
	Constraint *c = new Constraint(LineTangentConstraint,this,obj);
	constraints.push_back(c);
}

void ConstrainedObject::SetLineLength(double length)
{
	Constraint *c = GetExisting(LineLengthConstraint);
	if(c)
	{
		c->m_length = length;
	}
}

void ConstrainedObject::SetLineVerticalLength(double length)
{
	Constraint *c = GetExisting(LineVerticalLengthConstraint);
	if(c)
	{
		c->m_length = length;
	}
}

void ConstrainedObject::SetLineHorizontalLength(double length)
{
	Constraint *c = GetExisting(LineHorizontalLengthConstraint);
	if(c)
	{
		c->m_length = length;
	}
}

void ConstrainedObject::SetRadius(double radius)
{
	Constraint *c = GetExisting(RadiusConstraint);
	if(c)
	{
		c->m_length = radius;
	}
}

void ConstrainedObject::SetPointOnLineConstraint(HPoint* obj)
{
	if(RemoveExisting(obj,PointOnLineConstraint)) return;

	Constraint *c = new Constraint(PointOnLineConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetPointOnLineMidpointConstraint(HPoint* obj)
{
	if(RemoveExisting(obj,PointOnLineMidpointConstraint)) return;

	Constraint *c = new Constraint(PointOnLineMidpointConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetPointOnArcMidpointConstraint(HPoint* obj)
{
	if(RemoveExisting(obj,PointOnArcMidpointConstraint)) return;

	Constraint *c = new Constraint(PointOnArcMidpointConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetPointOnArcConstraint(HPoint* obj)
{
	if(RemoveExisting(obj,PointOnArcConstraint)) return;

	Constraint *c = new Constraint(PointOnArcConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
}

void ConstrainedObject::SetPointOnCircleConstraint(HPoint* obj)
{
	if(RemoveExisting(obj,PointOnCircleConstraint)) return;

	Constraint *c = new Constraint(PointOnCircleConstraint,this,obj);
	constraints.push_back(c);
	obj->constraints.push_back(c);
}
