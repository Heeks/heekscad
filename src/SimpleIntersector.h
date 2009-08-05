// SimpleIntersector.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class SimpleIntersector: public Intersector{
public:
	std::map<MyLine*, std::vector<Intersection> > Intersect(std::vector<MyLine> &lines);
	IntResult Intersects(MyLine* line1, MyLine* line2);
};
