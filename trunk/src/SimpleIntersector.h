// SimpleIntersector.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class SimpleIntersector: public Intersector{
public:
	std::map<FastCurve*, std::vector<Intersection> > Intersect(std::vector<FastCurve*> &lines);
	IntResult Intersects(FastLine* line1, FastLine* line2);
};
