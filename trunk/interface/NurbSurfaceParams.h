// NurbSurfaceParams.h

#pragma once

// structure for getting data from HeeksCAD faces

class CNurbSurfaceParams
{
public:
	int u_order;			/// U curve order
	int v_order;			/// V curve order
	int n_u_vertices;		/// U direction
	int n_v_vertices;		/// V direction
	int vertex_size;		/// = 3 for non-rational  = 4 for rational
	bool rational;
	double* vertex;			/// double array which holds the vertices
	int form;				/// 
	int n_u_knots;			/// Number of Knot values in U direction
	int n_v_knots;			/// Number of Knot values in U direction
	double* u_knot;			/// Knot vectors in U direction
	double* v_knot;			/// Knot vectors in V direction
	int u_knot_type;
	int v_knot_type;
	bool is_u_periodic;		/// true if surface is periodic in U direction.
	bool is_v_periodic;		/// true if surface is periodic in V direction.
	bool is_u_closed;
	bool is_v_closed;
	int self_intersecting;
	int convexity;

	CNurbSurfaceParams()
	{
		u_order = 0;
		v_order = 0;
		n_u_vertices = 0;
		n_v_vertices = 0;
		vertex_size = 0;
		rational = false;
		vertex = NULL;
		form = 0;
		n_u_knots = 0;
		n_v_knots = 0;
		u_knot = NULL;
		v_knot = NULL;
		u_knot_type = 0;
		v_knot_type = 0;
		is_u_periodic = false;
		is_v_periodic = false;
		is_u_closed = false;
		is_v_closed = false;
		self_intersecting = 0;
		convexity = 0;
	}

	~CNurbSurfaceParams()
	{
		if(vertex)free(vertex);
		if(u_knot)free(u_knot);
		if(v_knot)free(v_knot);
	}

};
