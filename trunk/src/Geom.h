// Geom.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

// functions to find intersections between gp items
bool intersect(const gp_Lin& lin, const gp_Lin& lin2, gp_Pnt &pnt);
bool intersect(const gp_Lin& lin, const gp_Pln& pln, gp_Pnt &pnt);
bool intersect(const gp_Pln& pln, const gp_Pln& pln2, gp_Lin& lin);
bool intersect(const gp_Pnt& pnt, const gp_Pln& pln);
bool intersect(const gp_Pnt& pnt, const gp_Lin& lin);
bool intersect(const gp_Pnt& pnt, const gp_Circ& cir);
void intersect(const gp_Lin& lin, const gp_Elips& elips, std::list<gp_Pnt> &list);
void intersect(const gp_Lin& lin, const gp_Circ& cir, std::list<gp_Pnt> &list);
void intersect(const gp_Circ& c1, const gp_Circ& c2, std::list<gp_Pnt> &list);

void extract(const gp_Pnt &p, double *m);
void extract(const gp_Vec &v, double *m);
void extract(const gp_XYZ &xyz, double *m);
void extract(const gp_Trsf& tr, double *m);
void extract_transposed(const gp_Trsf& tr, double *m);

#define TwoCircleType 100000

class PointLineOrCircle{
public:
	int type; // UnknownType, PointType, LineType, CircleType or TwoCircleType
	gp_Pnt p;
	gp_Lin l;
	gp_Circ c;
	gp_Circ c2;

	PointLineOrCircle() :type(0){}
};

gp_Pnt ClosestPointOnPlane(const gp_Pln& pln, const gp_Pnt &p);
gp_Pnt ClosestPointOnLine(const gp_Lin& line, const gp_Pnt &p);
void ClosestPointsOnLines(const gp_Lin& lin, const gp_Lin& lin2, gp_Pnt &p1, gp_Pnt &p2);// they might be the same point
void ClosestPointsLineAndCircle(const gp_Lin& lin, const gp_Circ& cir, std::list<gp_Pnt> &list);
double GetEllipseRotation(const gp_Elips& elips);
double DistanceToFoci(const gp_Pnt &pnt, const gp_Elips &elips);
void ClosestPointsLineAndEllipse(const gp_Lin& lin, const gp_Elips& elips, std::list<gp_Pnt> &list);

// I've made all the combinations of these, 3*3*3 = 27 :), but all except 10 are just to redirect
void TangentCircles(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Pnt& p1, const gp_Lin& l2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Pnt& p1, const gp_Lin& l2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Pnt& p1, const gp_Lin& l2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Pnt& p1, const gp_Circ& c2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Pnt& p1, const gp_Circ& c2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Pnt& p1, const gp_Circ& c2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Pnt& p2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Pnt& p2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Pnt& p2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Lin& l2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Lin& l2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Lin& l2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Circ& c2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Circ& c2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Lin& l1, const gp_Circ& c2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Pnt& p2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Pnt& p2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Pnt& p2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Lin& l2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Lin& l2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Lin& l2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Circ& c2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Circ& c2, const gp_Lin& l3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Circ& c2, const gp_Circ& c3, std::list<gp_Circ>& c_list);

void TangentCircles(const gp_Circ& c1, const gp_Circ& c2, const gp_Circ& c3, std::list<gp_Circ>& c_list);
void TangentCircles(const gp_Circ& c1, const gp_Lin& l2, const gp_Pnt& p3, std::list<gp_Circ>& c_list);
void TangentCircles(const PointLineOrCircle& plc1, const PointLineOrCircle& plc2, const PointLineOrCircle& plc3, std::list<gp_Circ>& c_list);
gp_Circ PointToCircle(const gp_Pnt& p);
bool LineToBigCircles(const gp_Lin& lin, const gp_Dir& z_axis, gp_Circ& c1, gp_Circ& c2);

gp_Pnt make_point(const double* p);
gp_Vec make_vector(const gp_Pnt &p1, const gp_Pnt &p2);
gp_Vec make_vector(const double* v);
gp_Lin make_line(const gp_Pnt &p1, const gp_Pnt &p2);
gp_Lin make_line(const gp_Pnt &p, const gp_Vec &v);

gp_Trsf make_matrix(const double* m);
gp_Trsf make_matrix(const gp_Pnt &origin, const gp_Vec &x_axis, const gp_Vec &y_axis);

void add_pnt_to_doubles(const gp_Pnt& pnt, std::list<double> &dlist);
int convert_pnts_to_doubles(const std::list<gp_Pnt> &plist, std::list<double> &dlist);
bool make_point_from_doubles(const std::list<double> &dlist, std::list<double>::const_iterator &It, gp_Pnt& pnt, bool four_doubles = false);
int convert_doubles_to_pnts(const std::list<double> &dlist, std::list<gp_Pnt> &plist, bool four_doubles = false);
int convert_gripdata_to_pnts(const std::list<GripData> &dlist, std::list<gp_Pnt> &plist);

bool IsEqual(gp_Ax2 ax1, gp_Ax2 ax2);
bool IsEqual(gp_Ax1 ax1, gp_Ax1 ax2);

