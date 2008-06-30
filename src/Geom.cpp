// Geom.cpp

#include "stdafx.h"
#include "Geom.h"
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <GeomAPI_IntCS.hxx>
#include <GeomAPI_IntSS.hxx>

void intersect(const gp_Lin& lin, const gp_Lin& lin2, std::list<gp_Pnt> &list)
{
	// if the lines are parallel, list contains no points
	// else list contains two points, the first is on lin, and the second on lin2
	gp_Dir d1(lin.Direction());
	gp_Dir d2(lin2.Direction());
	gp_Vec v3(lin.Location(), lin2.Location());

	gp_Vec xp = gp_Vec(d1) ^ gp_Vec(d2);

	gp_Trsf mat;
	mat.SetValues(d1.X(), -d2.X(), xp.X(), 0,      d1.Y(), -d2.Y(), xp.Y(), 0,     d1.Z(), -d2.Z(), xp.Z(), 0,     0.0001, 0.00000001);

	gp_Vec v4 = v3.Transformed(mat.Inverted());
	list.push_back(gp_Pnt(lin.Location().XYZ() + d1.XYZ() * v4.X()));
	list.push_back(gp_Pnt(lin2.Location().XYZ() + d2.XYZ() * v4.Y()));
}

void intersect(const gp_Lin& lin, const gp_Pln& pln, std::list<gp_Pnt> &list)
{
	Handle (Geom_Line) g_l = new Geom_Line(lin);
	Handle (Geom_Plane) g_p = new Geom_Plane(pln);

	{
		Handle (Geom_Curve) hc = Handle (Geom_Line)::DownCast(g_l);
		Handle (Geom_Surface) hs = Handle (Geom_Plane)::DownCast(g_p);

		GeomAPI_IntCS intf(hc, hs);
		if(intf.IsDone()){
			int np = intf.NbPoints();
			for(int i = 1; i<=np; i++){
				gp_Pnt p = intf.Point(i);
				list.push_back(p);
			}
		}
	}
}

bool intersect(const gp_Pln& pln, const gp_Pln& pln2, gp_Lin& lin)
{
	bool result = false;
	{
		Handle (Geom_Plane) g_p = new Geom_Plane(pln);
		Handle (Geom_Plane) g_p2 = new Geom_Plane(pln2);

		{
			Handle (Geom_Surface) hs = Handle (Geom_Plane)::DownCast(g_p);
			Handle (Geom_Surface) hs2 = Handle (Geom_Plane)::DownCast(g_p2);

			{
				GeomAPI_IntSS intf(hs, hs2, wxGetApp().m_geom_tol);
				if(intf.IsDone()){
					int nl = intf.NbLines();
					if(nl>0){
						Handle (Geom_Curve) curve = intf.Line(1);
						lin = ((Handle(Geom_Line)::DownCast(curve)))->Lin();
						result = true;
					}
				}
			}
		}
	}

	return result;
}

void extract(const gp_Pnt &p, double *m)
{
	m[0] = p.X();
	m[1] = p.Y();
	m[2] = p.Z();
}

void extract(const gp_Vec &v, double *m)
{
	m[0] = v.X();
	m[1] = v.Y();
	m[2] = v.Z();
}

void extract(const gp_XYZ &xyz, double *m)
{
	m[0] = xyz.X();
	m[1] = xyz.Y();
	m[2] = xyz.Z();
}

void extract(const gp_Trsf& tr, double *m)
{
	m[0] = tr.Value(1, 1);
	m[1] = tr.Value(1, 2);
	m[2] = tr.Value(1, 3);
	m[3] = tr.Value(1, 4);
	m[4] = tr.Value(2, 1);
	m[5] = tr.Value(2, 2);
	m[6] = tr.Value(2, 3);
	m[7] = tr.Value(2, 4);
	m[8] = tr.Value(3, 1);
	m[9] = tr.Value(3, 2);
	m[10] = tr.Value(3, 3);
	m[11] = tr.Value(3, 4);
	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
}

void extract_transposed(const gp_Trsf& tr, double *m)
{
	m[0] = tr.Value(1, 1);
	m[1] = tr.Value(2, 1);
	m[2] = tr.Value(3, 1);
	m[3] = 0;
	m[4] = tr.Value(1, 2);
	m[5] = tr.Value(2, 2);
	m[6] = tr.Value(3, 2);
	m[7] = 0;
	m[8] = tr.Value(1, 3);
	m[9] = tr.Value(2, 3);
	m[10] = tr.Value(3, 3);
	m[11] = 0;
	m[12] = tr.Value(1, 4);
	m[13] = tr.Value(2, 4);
	m[14] = tr.Value(3, 4);
	m[15] = 1;
}

gp_Pnt ClosestPointOnPlane(const gp_Pln& pln, const gp_Pnt &p){
	// returns closest vertex on plane to point pt 
	gp_Vec n = pln.Axis().Direction().XYZ();
	gp_Pnt l = pln.Location();

	gp_Pnt rp = p.XYZ() + (n * (n * gp_Vec(l.XYZ()) - n * gp_Vec(p.XYZ()))).XYZ();
	return rp;
}

gp_Pnt make_point(const double* p)
{
	return gp_Pnt(p[0], p[1], p[2]);
}

gp_Lin make_line(const gp_Pnt &p1, const gp_Pnt &p2)
{
    gp_Dir d(p2.XYZ() - p1.XYZ());
	return gp_Lin(p1, d);
}

gp_Lin make_line(const gp_Pnt &p, const gp_Vec &v)
{
    gp_Dir d(v.XYZ());
	return gp_Lin(p, d);
}

gp_Vec make_vector(const gp_Pnt &p1, const gp_Pnt &p2)
{
	return gp_Vec(p2.XYZ() - p1.XYZ());
}

gp_Vec make_vector(const double* v)
{
	return gp_Vec(v[0], v[1], v[2]);
}

gp_Trsf make_matrix(const double* m)
{
	gp_Trsf tr;
	tr.SetValues(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], 0.0001, 0.00000001);
	return tr;
}

gp_Trsf make_matrix(const gp_Pnt &origin, const gp_Vec &x_axis, const gp_Vec &y_axis)
{
	gp_Vec unit_x = x_axis.Normalized();

    double t = unit_x.X() * y_axis.X()
		+ unit_x.Y() * y_axis.Y()
		+ unit_x.Z() * y_axis.Z() ;
    gp_Vec y_orthogonal(y_axis.X() - unit_x.X() * t, y_axis.Y() - unit_x.Y() * t, y_axis.Z() - unit_x.Z() * t);

	gp_Vec unit_y = y_orthogonal.Normalized();
	gp_Vec unit_z = (unit_x ^ y_orthogonal).Normalized();

	double m[16] = {unit_x.X(), unit_y.X(), unit_z.X(), origin.X(), unit_x.Y(), unit_y.Y(), unit_z.Y(), origin.Y(), unit_x.Z(), unit_y.Z(), unit_z.Z(), origin.Z(), 0, 0, 0, 1};
	return make_matrix(m);
}

gp_Pnt transform_point(const gp_Trsf& tr, const gp_Pnt &p)
{
	gp_XYZ xyz = p.XYZ();
	tr.Transforms(xyz);
    return gp_Pnt(xyz);
}

gp_Vec transform_vector(const gp_Trsf& tr, const gp_Vec &v)
{
	gp_XYZ xyz = v.XYZ();
	tr.Transforms(xyz);
    return gp_Vec(xyz);
}

gp_Pln transform(const gp_Trsf& tr, const gp_Pln &p)
{
	return p.Transformed(tr);
}

