// BentleyOttmann.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Intersector.h"

class BentleyOttmann: public Intersector{
public:
	std::map<FastCurve*, std::vector<Intersection> > Intersect(std::vector<FastCurve*> &lines);

	enum EventType
	{
		AddType,
		RemoveType,
		AddRemoveType,
		IntersectionType
	};
};

bool MyIsEqual(double a, double b);
bool MyIsEqual2(FastLine* line1, FastLine* line2, double at);
double MyRound(double d);
IntResult Intersects(FastLine* line1, FastLine* line2);
void InsertEvent(BentleyOttmann::EventType type, double x, FastLine* line);


