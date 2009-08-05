// BentleyOttmann.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

#include "Intersector.h"

class BentleyOttmann: public Intersector{
public:
	std::map<MyLine*, std::vector<Intersection> > Intersect(std::vector<MyLine> &lines);
};

enum EventType
{
	AddType,
	RemoveType,
	AddRemoveType,
	IntersectionType
};

bool MyIsEqual(double a, double b);
bool MyIsEqual2(MyLine* line1, MyLine* line2, double at);
double MyRound(double d);
IntResult Intersects(MyLine* line1, MyLine* line2);
void InsertEvent(EventType type, double x, MyLine* line);

