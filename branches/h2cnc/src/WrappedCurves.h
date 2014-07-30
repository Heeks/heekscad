// WrappedCurves.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

enum WhichEnd
{
	FirstEnd,
	LastEnd,
	NoEnd
};

enum WhichPoint
{
	PointA,
	PointB
};

class BoundedCurve
{
public:
	FastCurve* line;
	double startu,endu;
	double m_tol;
	BoundedCurve(FastCurve *line, double startu, double endu, double tol);
	void Reverse();
	double GetAX();
	double GetAY();
	double GetX(double u);
	double GetY(double u);
	double GetBX();
	double GetBY();
	gp_Pnt GetMidPoint();
	std::vector<RayIntersection> RayIntersects(gp_Pnt pnt);
	gp_Pnt Begin();
	gp_Pnt End();
};

class CompoundSegment
{
public:
	BoundedCurve *firstline;
	WhichPoint firstpoint;
	BoundedCurve *lastline;
	WhichPoint lastpoint;
	std::list<BoundedCurve*> lines;
	std::vector<WhichPoint> points;
	double m_tol;
	CompoundSegment();
	CompoundSegment(FastCurve *line, double tol, double startu, double endu);
	~CompoundSegment();
	void Reverse();
	WhichEnd GetWhichEnd(double atx, double aty);
	gp_Pnt Begin();
	gp_Pnt End();
	double GetArea(BoundedCurve* c1, WhichPoint dir1, BoundedCurve* c2, WhichPoint dir2);
	void GetEdges(std::list<TopoDS_Edge>& edges);
	void DrawDebugLine(gp_Pnt A, gp_Pnt B, int i);
	void render_text(const wxChar* str);
	double GetArea();
	int GetRayIntersectionCount(gp_Pnt pnt);
	int GetRayIntersectionCount(gp_Pnt pnt, BoundedCurve* without);
	bool GetCW();
	void Order();
	void Add(CompoundSegment* seg, double atx, double aty);
};
