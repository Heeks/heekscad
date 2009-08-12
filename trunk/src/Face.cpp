// Face.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Face.h"
#include "../interface/NurbSurfaceParams.h"
#include <Geom_BSplineSurface.hxx>
#include <TColgp_Array2OfPnt.hxx>

CFace::CFace(const TopoDS_Face &face):m_topods_face(face), m_temp_attr(0){
#if _DEBUG
	gp_Pnt pos;
	gp_Dir norm = GetNormalAtUV(0, 0, &pos);
	m_pos_x = pos.X();
	m_pos_y = pos.Y();
	m_pos_z = pos.Z();
	m_normal_x = norm.X();
	m_normal_y = norm.Y();
	m_normal_z = norm.Z();
	m_orientation = Orientation();
#endif
}

CFace::~CFace(){
}

static Standard_Boolean TriangleIsValid(const gp_Pnt& P1, const gp_Pnt& P2, const gp_Pnt& P3)
{ 
  gp_Vec V1(P1,P2);								// V1=(P1,P2)
  gp_Vec V2(P2,P3);								// V2=(P2,P3)
  gp_Vec V3(P3,P1);								// V3=(P3,P1)
  
  if ((V1.SquareMagnitude() > 1.e-10) && (V2.SquareMagnitude() > 1.e-10) && (V3.SquareMagnitude() > 1.e-10))
    {
      V1.Cross(V2);								// V1 = Normal	
      if (V1.SquareMagnitude() > 1.e-10)
	return Standard_True;
      else
	return Standard_False;
    }
  else
    return Standard_False;
  
}

void CFace::glCommands(bool select, bool marked, bool no_color){
	bool owned_by_solid = false;
	if(GetParentBody()) {
		// using existing BRepMesh::Mesh
		// use solid's colour
		owned_by_solid = true;
	}
	else {
		// clean mesh
		double pixels_per_mm = wxGetApp().GetPixelScale();
		BRepTools::Clean(m_topods_face);
		BRepMesh::Mesh(m_topods_face, 1/pixels_per_mm);

		// use a default material
		Material().glMaterial(1.0);
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
	}

	glBegin(GL_TRIANGLES);

	{
		StdPrs_ToolShadedShape SST;

		// Get triangulation
		TopLoc_Location L;
		Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(m_topods_face,L);
		gp_Trsf tr = L;

		if(!facing.IsNull()){
			Poly_Connect pc(facing);	
			const TColgp_Array1OfPnt& Nodes = facing->Nodes();
			const Poly_Array1OfTriangle& triangles = facing->Triangles();
			TColgp_Array1OfDir myNormal(Nodes.Lower(), Nodes.Upper());

			SST.Normal(m_topods_face, pc, myNormal);
			double Umin, Umax, Vmin, Vmax;
			BRepTools::UVBounds(m_topods_face,Umin, Umax, Vmin, Vmax);

			Standard_Integer nnn = facing->NbTriangles();					// nnn : nombre de triangles
			Standard_Integer nt, n1, n2, n3 = 0;						// nt  : triangle courant
			// ni  : sommet i du triangle courant
			for (nt = 1; nt <= nnn; nt++)					
			{
				if (SST.Orientation(m_topods_face) == TopAbs_REVERSED)			// si la face est "reversed"
					triangles(nt).Get(n1,n3,n2);						// le triangle est n1,n3,n2
				else 
					triangles(nt).Get(n1,n2,n3);						// le triangle est n1,n2,n3

				if (TriangleIsValid (Nodes(n1),Nodes(n2),Nodes(n3)) )
				{
					gp_Pnt v1 = Nodes(n1).Transformed(tr);
					gp_Pnt v2 = Nodes(n2).Transformed(tr);
					gp_Pnt v3 = Nodes(n3).Transformed(tr);

					glNormal3f((float)(myNormal(n1).X()), (float)(myNormal(n1).Y()), (float)(myNormal(n1).Z()));
					glVertex3f((float)(v1.X()), (float)(v1.Y()), (float)(v1.Z()));
					glNormal3f((float)(myNormal(n2).X()), (float)(myNormal(n2).Y()), (float)(myNormal(n2).Z()));
					glVertex3f((float)(v2.X()), (float)(v2.Y()), (float)(v2.Z()));
					glNormal3f((float)(myNormal(n3).X()), (float)(myNormal(n3).Y()), (float)(myNormal(n3).Z()));
					glVertex3f((float)(v3.X()), (float)(v3.Y()), (float)(v3.Z()));
				}
			}
		}
	}

	glEnd();

	if(!owned_by_solid)
	{
		glDisable(GL_LIGHTING);
		glShadeModel(GL_FLAT);
	}
}

void CFace::GetBox(CBox &box){
	if(!m_box.m_valid)
	{
		// there must be a better way than re-using the render code
		// Get triangulation
		if(GetParentBody() == NULL){
			BRepTools::Clean(m_topods_face);
			BRepMesh::Mesh(m_topods_face, 1.0);
		}
		TopLoc_Location L;
		Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(m_topods_face,L);
		gp_Trsf tr = L;
		const Poly_Array1OfTriangle &triangles = facing->Triangles();
		const TColgp_Array1OfPnt & nodes = facing->Nodes();
		if (!facing.IsNull())
		{
			for ( int i=facing->NbTriangles(); i >= 1; --i ) // (indeksy 1...N)
			{
				Poly_Triangle triangle = triangles(i);

				Standard_Integer node1,node2,node3;
				triangle.Get(node1, node2, node3);

				gp_Pnt v1 = nodes(node1);
				gp_Pnt v2 = nodes(node2);
				gp_Pnt v3 = nodes(node3);

				v1.Transform(tr);
				v2.Transform(tr);
				v3.Transform(tr);
				m_box.Insert(v1.X(), v1.Y(), v1.Z());
				m_box.Insert(v2.X(), v2.Y(), v2.Z());
				m_box.Insert(v3.X(), v3.Y(), v3.Z());
			}
		}
	}

	box.Insert(m_box);
}

bool CFace::ModifyByMatrix(const double *m){
	if(GetParentBody() == NULL)
	{
		gp_Trsf mat = make_matrix(m);
		BRepBuilderAPI_Transform myBRepTransformation(m_topods_face,mat);
		TopoDS_Shape new_shape = myBRepTransformation.Shape();
		wxGetApp().Add(new CFace(*((TopoDS_Face*)(&new_shape))), NULL);
		wxGetApp().DeleteUndoably(this);
	}
	return true;
}

void CFace::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal){
	if(GetParentBody()) {
		// using existing BRepMesh::Mesh
	}
	else {
		BRepTools::Clean(m_topods_face);
		BRepMesh::Mesh(m_topods_face, cusp);
	}

	StdPrs_ToolShadedShape SST;

	// Get triangulation
	TopLoc_Location L;
	Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(m_topods_face,L);
	gp_Trsf tr = L;

	if(!facing.IsNull()){
		Poly_Connect pc(facing);	
		const TColgp_Array1OfPnt& Nodes = facing->Nodes();
		const Poly_Array1OfTriangle& triangles = facing->Triangles();
		TColgp_Array1OfDir myNormal(Nodes.Lower(), Nodes.Upper());

		SST.Normal(m_topods_face, pc, myNormal);
		double Umin, Umax, Vmin, Vmax;
		BRepTools::UVBounds(m_topods_face,Umin, Umax, Vmin, Vmax);

		Standard_Integer nnn = facing->NbTriangles();					// nnn : nombre de triangles
		Standard_Integer nt, n1, n2, n3 = 0;						// nt  : triangle courant

		double x[9], n[9];

		// ni  : sommet i du triangle courant
		for (nt = 1; nt <= nnn; nt++)					
		{
			if (SST.Orientation(m_topods_face) == TopAbs_REVERSED)			// si la face est "reversed"
				triangles(nt).Get(n1,n3,n2);						// le triangle est n1,n3,n2
			else 
				triangles(nt).Get(n1,n2,n3);						// le triangle est n1,n2,n3

			if (TriangleIsValid (Nodes(n1),Nodes(n2),Nodes(n3)) )
			{
				gp_Pnt v1 = Nodes(n1).Transformed(tr);
				gp_Pnt v2 = Nodes(n2).Transformed(tr);
				gp_Pnt v3 = Nodes(n3).Transformed(tr);


				x[0] = v1.X();
				x[1] = v1.Y();
				x[2] = v1.Z();
				x[3] = v2.X();
				x[4] = v2.Y();
				x[5] = v2.Z();
				x[6] = v3.X();
				x[7] = v3.Y();
				x[8] = v3.Z();

				if(just_one_average_normal)
				{
					gp_Vec V1(v1, v2);
					gp_Vec V2(v1, v3);
					extract((V1 ^ V2).Normalized(), n);
				}
				else
				{
					n[0] = myNormal(n1).X();
					n[1] = myNormal(n1).Y();
					n[2] = myNormal(n1).Z();
					n[3] = myNormal(n2).X();
					n[4] = myNormal(n2).Y();
					n[5] = myNormal(n2).Z();
					n[6] = myNormal(n3).X();
					n[7] = myNormal(n3).Y();
					n[8] = myNormal(n3).Z();
				}
				(*callbackfunc)(x, n);
			}
		}
	}
}

bool best_triangle_found = false;
double biggest_area;
double best_triangle_coords[9];
double best_triangle_normals[9];

void best_triangle_for_face(double *x, double *n)
{
	gp_Pnt p1(x[0], x[1], x[2]);
	gp_Pnt p2(x[3], x[4], x[5]);
	gp_Pnt p3(x[6], x[7], x[8]);
	gp_Vec v1(p1, p2);
	gp_Vec v2(p1, p3);
	double area = (v1 ^ v2).Magnitude(); // cross product is bigger for most right-angled angle
	if(!best_triangle_found || area > biggest_area){
		biggest_area = area;
		memcpy(best_triangle_coords, x, 9*sizeof(double));
		memcpy(best_triangle_normals, n, 9*sizeof(double));
		best_triangle_found = true;
	}
}

gp_Dir CFace::GetMiddleNormal(gp_Pnt *pos)const{
	// get bounds of face
	Standard_Real umin, umax, vmin, vmax;
	BRepTools::UVBounds(m_topods_face, umin, umax, vmin, vmax); 
	return GetNormalAtUV((umin + umax)/2, (vmin + vmax)/2, pos);
}

gp_Dir CFace::GetNormalAtUV(double u, double v, gp_Pnt *pos)const{
	if(m_topods_face.IsNull()) return gp_Dir(0, 0, 1);
	Handle(Geom_Surface) surf=BRep_Tool::Surface(m_topods_face);          // get surface properties
	GeomLProp_SLProps props(surf, u, v, 1, 0.01);          // get surface normal
	if(!props.IsNormalDefined())return gp_Dir(0, 0, 1);
	gp_Dir norm=props.Normal();                         // check orientation
	if(pos)*pos = props.Value();
	if(m_topods_face.Orientation()==TopAbs_REVERSED) norm.Reverse();
	return norm;
}

bool CFace::GetUVAtPoint(const gp_Pnt &pos, double *u, double *v)const{
	Handle(Geom_Surface) surface = BRep_Tool::Surface(m_topods_face);
	GeomAPI_ProjectPointOnSurf projection( pos, surface);

	if(projection.NbPoints() > 0)
	{               
		if(projection.LowerDistance() < 0.01 )
		{
			projection.LowerDistanceParameters(*u, *v);
			return true;
		}
	}     
	return false;
}

bool CFace::GetClosestPoint(const gp_Pnt &pos, gp_Pnt &closest_pnt)const{
	Handle(Geom_Surface) surface = BRep_Tool::Surface(m_topods_face);
	GeomAPI_ProjectPointOnSurf projection( pos, surface);

	if(projection.NbPoints() > 0)
	{               
		closest_pnt = projection.NearestPoint();
		return true;
	}     
	return false;
}

double CFace::Area()const{
	GProp_GProps System;
	BRepGProp::SurfaceProperties(m_topods_face,System);
	return System.Mass();
}

void CFace::WriteXML(TiXmlNode *root)
{
	CShape::m_solids_found = true;
}

void CFace::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyString(_("surface type"), GetSurfaceTypeStr(), NULL));

	HeeksObj::GetProperties(list);
}

static CFace* face_for_tools = NULL;

void FaceToSketchTool::Run(){
	CSketch* new_object = new CSketch();
	ConvertFaceToSketch2(face_for_tools->Face(), new_object, deviation);
	wxGetApp().AddUndoably(new_object, NULL, NULL);
}

double FaceToSketchTool::deviation = 0.1;

static FaceToSketchTool make_sketch_tool;

class MakeCoordSystem:public Tool
{
	// only use this if GetSurfaceType() == GeomAbs_Plane
public:
	const wxChar* GetTitle(){return _("Make Coordinate System");}
	wxString BitmapPath(){return _T("coordsys");}
	void Run(){
		gp_Pln plane;
		face_for_tools->GetPlaneParams(plane);
		CoordinateSystem* new_object = new CoordinateSystem(_("Face Coordinate System"), plane.Location(), plane.XAxis().Direction(), plane.YAxis().Direction());
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().m_marked_list->Clear(true);
		wxGetApp().m_marked_list->Add(new_object, true);
		wxGetApp().Repaint();
	}
};

static MakeCoordSystem make_coordsys;

void CFace::GetTools(std::list<Tool*>* t_list, const wxPoint* p){
	face_for_tools = this;
	t_list->push_back(&make_sketch_tool);
	if(GetSurfaceType() == GeomAbs_Plane)t_list->push_back(&make_coordsys);
}

int CFace::GetSurfaceType()
{
	// enum GeomAbs_SurfaceType
	// 0 - GeomAbs_Plane
	// 1 - GeomAbs_Cylinder
	// 2 - GeomAbs_Cone
	// 3 - GeomAbs_Sphere
	// 4 - GeomAbs_Torus
	// 5 - GeomAbs_BezierSurface
	// 6 - GeomAbs_BSplineSurface
	// 7 - GeomAbs_SurfaceOfRevolution
	// 8 - GeomAbs_SurfaceOfExtrusion
	// 9 - GeomAbs_OffsetSurface
	// 10- GeomAbs_OtherSurface

	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	GeomAbs_SurfaceType surface_type = surface.GetType();
	return surface_type;
}

wxString CFace::GetSurfaceTypeStr()
{
	wxString surface_type = _("unknown");
	switch(GetSurfaceType())
	{
	case GeomAbs_Plane:
		surface_type = _("plane");
		break;

	case GeomAbs_Cylinder:
		surface_type = _("cylinder");
		break;

	case GeomAbs_Cone:
		surface_type = _("cone");
		break;

	case GeomAbs_Sphere:
		surface_type = _("sphere");
		break;

	case GeomAbs_Torus:
		surface_type = _("torus");
		break;

	case GeomAbs_BezierSurface:
		surface_type = _("bezier");
		break;

	case GeomAbs_BSplineSurface:
		surface_type = _("bspline");
		break;

	case GeomAbs_SurfaceOfRevolution:
		surface_type = _("revolution");
		break;

	case GeomAbs_SurfaceOfExtrusion:
		surface_type = _("extrusion");
		break;

	case GeomAbs_OffsetSurface:
		surface_type = _("offset");
		break;
	}

	return surface_type;
}

void CFace::GetPlaneParams(gp_Pln &p)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	p = surface.Plane();
}

void CFace::GetCylinderParams(gp_Cylinder &c)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	c = surface.Cylinder();
}

void CFace::GetSphereParams(gp_Sphere &s)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	s = surface.Sphere();
}

void CFace::GetConeParams(gp_Cone &c)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	c = surface.Cone();
}

bool CFace::GetNurbSurfaceParams(CNurbSurfaceParams* params)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	params->u_order = surface.UDegree();
	params->v_order = surface.VDegree();
	params->n_u_vertices = surface.NbUPoles();
	params->n_v_vertices = surface.NbVPoles();
	params->rational = surface.IsURational() != 0;
	params->is_u_periodic = surface.IsUPeriodic() != 0;
	params->is_v_periodic = surface.IsVPeriodic() != 0;
	params->is_u_closed = surface.IsUClosed() != 0;
	params->is_v_closed = surface.IsVClosed() != 0;

	Handle(Geom_Surface) aGeomSurface = BRep_Tool::Surface(m_topods_face);
	if(aGeomSurface->DynamicType() != STANDARD_TYPE(Geom_BSplineSurface))
		return false;

	Handle(Geom_BSplineSurface) aBSplineSurface = Handle(Geom_BSplineSurface)::DownCast(aGeomSurface);

	int myNbUPoles = aBSplineSurface->NbUPoles();
	int myNbVPoles = aBSplineSurface->NbVPoles();

	TColgp_Array2OfPnt aPoles(1, myNbVPoles, 1, myNbUPoles);

	TColStd_Array1OfReal aVKnots(1, aBSplineSurface->NbVKnots());
	TColStd_Array1OfReal aUKnots(1, aBSplineSurface->NbUKnots());

	aBSplineSurface->Poles(aPoles);
	aBSplineSurface->UKnots(aUKnots);
	aBSplineSurface->VKnots(aVKnots);

	//Push the nurbs in Coin3d

	// Control Point
	Standard_Integer i, j, aCounter;
	params->vertex_size = 4;
	int nVertexDoubles = myNbVPoles*myNbUPoles*params->vertex_size; //Create array of control points and their weights
	params->vertex = new double[nVertexDoubles];

	aCounter = -1;

	for(j = 1; j <= myNbVPoles; j++) {
		for(i = 1; i <= myNbUPoles; i++) {
			const gp_Pnt& aPoint = aBSplineSurface->Pole(i, j); //Control point (U,V)
			params->vertex[++aCounter] = aPoint.X();
			params->vertex[++aCounter] = aPoint.Y();
			params->vertex[++aCounter] = aPoint.Z();
			params->vertex[++aCounter] = aBSplineSurface->Weight(i, j);
		}
	}

	std::list<double> knot_list;

	try {
		//Fill the knot`s array taking into account multiplicities
		// VKnots
		for(i = aVKnots.Lower(); i<=aVKnots.Upper(); i++) {
			for(j = 1; j<= aBSplineSurface->VMultiplicity(i); j++)
				knot_list.push_back(aVKnots(i));
		}

		params->v_knot = new double[knot_list.size()];

		aCounter = -1;
		for(std::list<double>::iterator It = knot_list.begin(); It != knot_list.end(); It++)
		{
			params->v_knot[++aCounter] = *It;
		}
		params->n_v_knots = aCounter+1;

		// UKnots
		knot_list.clear();
		for(i = aUKnots.Lower(); i<=aUKnots.Upper(); i++) {
			for(j = 1; j<= aBSplineSurface->UMultiplicity(i); j++)
				knot_list.push_back(aUKnots(i));
		}

		params->u_knot = new double[knot_list.size()];

		aCounter = -1;
		for(std::list<double>::iterator It = knot_list.begin(); It != knot_list.end(); It++)
		{
			params->u_knot[++aCounter] = *It;
		}
		params->n_u_knots = aCounter+1;
	}
	catch(Standard_Failure) {
		return false;
	}

	return true;
}

CEdge* CFace::GetFirstEdge()
{
	if (m_edges.size()==0) return NULL;
	m_edgeIt = m_edges.begin();
	return *m_edgeIt;
}

CEdge* CFace::GetNextEdge()
{
	if (m_edges.size()==0 || m_edgeIt==m_edges.end()) return NULL;
	m_edgeIt++;
	if (m_edgeIt==m_edges.end()) return NULL;
	return *m_edgeIt;
}

CLoop* CFace::GetFirstLoop()
{
	if (m_loops.size()==0) return NULL;
	m_loopIt = m_loops.begin();
	return *m_loopIt;
}

CLoop* CFace::GetNextLoop()
{
	if (m_loops.size()==0 || m_loopIt==m_loops.end()) return NULL;
	m_loopIt++;
	if (m_loopIt==m_loops.end()) return NULL;
	return *m_loopIt;
}

bool CFace::Orientation()
{
	TopAbs_Orientation o = m_topods_face.Orientation();
	return (o == TopAbs_FORWARD);
}

void CFace::GetUVBox(double *uv_box)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	uv_box[0] = surface.FirstUParameter();
	uv_box[1] = surface.LastUParameter();
	uv_box[2] = surface.FirstVParameter();
	uv_box[3] = surface.LastVParameter();
}

void CFace::GetSurfaceUVPeriod(double *uv)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	uv[0] = surface.UPeriod();
	uv[1] = surface.VPeriod();
}

CShape* CFace::GetParentBody()
{
	if(Owner() == NULL)return NULL;
	if(Owner()->Owner() == NULL)return NULL;
	if(Owner()->Owner()->GetType() != SolidType)return NULL;
	return (CShape*)(Owner()->Owner());
}
