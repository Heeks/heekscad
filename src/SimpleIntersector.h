// SimpleIntersector.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

class SimpleIntersector: public Intersector{
public:
	std::map<FastCurve*, std::vector<Intersection> > Intersect(std::vector<FastCurve*> &lines);
	std::vector<IntResult> LinesIntersect(FastLine* line1, FastLine* line2);
	std::vector<IntResult> LineArcIntersect(FastLine* line, FastArc* arc);
	std::vector<IntResult> ArcsIntersect(FastArc* arc1, FastArc* arc2);
	IntResult ArcsIntersect(FastArc* arc1, FastArc* arc2, double x, double y);
	IntResult LineArcIntersect(FastLine* line, FastArc* arc, double u);
	std::vector<IntResult> Intersects(FastCurve* curve1, FastCurve* curve2);
};
