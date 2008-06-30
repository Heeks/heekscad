// Geom.h

#pragma once

static const double Pi = 3.14159265358979323846264338327950288419716939937511;

// functions to find intersections between gp items

void intersect(const gp_Lin& lin, const gp_Lin& lin2, std::list<gp_Pnt> &list);
void intersect(const gp_Lin& lin, const gp_Pln& pln, std::list<gp_Pnt> &list);
bool intersect(const gp_Pln& pln, const gp_Pln& pln2, gp_Lin& lin);

void extract(const gp_Pnt &p, double *m);
void extract(const gp_Vec &v, double *m);
void extract(const gp_XYZ &xyz, double *m);
void extract(const gp_Trsf& tr, double *m);
void extract_transposed(const gp_Trsf& tr, double *m);

gp_Pnt ClosestPointOnPlane(const gp_Pln& pln, const gp_Pnt &p);

gp_Pnt make_point(const double* p);
gp_Vec make_vector(const gp_Pnt &p1, const gp_Pnt &p2);
gp_Vec make_vector(const double* v);
gp_Lin make_line(const gp_Pnt &p1, const gp_Pnt &p2);
gp_Lin make_line(const gp_Pnt &p, const gp_Vec &v);

gp_Trsf make_matrix(const double* m);
gp_Trsf make_matrix(const gp_Pnt &origin, const gp_Vec &x_axis, const gp_Vec &y_axis);

