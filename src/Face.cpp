// Face.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Face.h"
#include "../interface/NurbSurfaceParams.h"
#include "FaceTools.h"
#include "Sketch.h"
#include "RuledSurface.h"
#include "HeeksFrame.h"
#include "InputModeCanvas.h"

CFace::CFace(const TopoDS_Face &face):m_topods_face(face), m_temp_attr(0){
#if _DEBUG
	gp_Pnt pos;
	gp_Dir norm = GetMiddleNormal(&pos);
	m_pos_x = pos.X();
	m_pos_y = pos.Y();
	m_pos_z = pos.Z();
	m_normal_x = norm.X();
	m_normal_y = norm.Y();
	m_normal_z = norm.Z();
	m_orientation = Orientation();
#endif
	m_marking_gl_list = 0;
}

CFace::~CFace(){
}

void CFace::glCommands(bool select, bool marked, bool no_color){
	bool owned_by_solid = false;
	if(GetParentBody()) {
		// using existing BRepMesh::Mesh
		// use solid's colour
		owned_by_solid = true;

		// add a marking display list
		if(m_marking_gl_list)
		{
			glCallList(m_marking_gl_list);
		}
	}
	else {
		// clean mesh
		double pixels_per_mm = wxGetApp().GetPixelScale();
		MeshFace(m_topods_face, 1/pixels_per_mm);

		// use a default material
		Material().glMaterial(1.0);
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
	}

	DrawFaceWithCommands(m_topods_face);

	if(!owned_by_solid)
	{
		glDisable(GL_LIGHTING);
		glShadeModel(GL_FLAT);
	}
}

CFace* FaceForBoxCallback;

void box_callback(const double* x, const double* n)
{
	FaceForBoxCallback->m_box.Insert(x[0],x[1],x[2]);
	FaceForBoxCallback->m_box.Insert(x[3],x[4],x[5]);
	FaceForBoxCallback->m_box.Insert(x[6],x[7],x[8]);
}

const wxBitmap &CFace::GetIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(wxGetApp().GetResFolder() + _T("/icons/face.png")));
	return *icon;
}

void CFace::GetBox(CBox &box){
//	if(!m_box.m_valid)
	{
		// there must be a better way than re-using the render code
		// Get triangulation
		if(GetParentBody() == NULL){
			MeshFace(m_topods_face,.01);
		}

		FaceForBoxCallback = this;
		DrawFace(m_topods_face,box_callback,false);
	}

	box.Insert(m_box);
}

void CFace::ModifyByMatrix(const double *m){
	if(GetParentBody() == NULL)
	{
		gp_Trsf mat = make_matrix(m);
		BRepBuilderAPI_Transform myBRepTransformation(m_topods_face,mat);
		m_topods_face = TopoDS::Face(myBRepTransformation.Shape());
	}
}

void CFace::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal){
	if(GetParentBody()) {
		// using existing BRepMesh::Mesh
	}
	else {
		MeshFace(m_topods_face,1/cusp);
	}

	DrawFace(m_topods_face,callbackfunc,just_one_average_normal);
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
	return GetFaceNormalAtUV(m_topods_face, u, v, pos);
}

bool CFace::GetUVAtPoint(const gp_Pnt &pos, double *u, double *v)const{
	Handle(Geom_Surface) surface = BRep_Tool::Surface(m_topods_face);
	GeomAPI_ProjectPointOnSurf projection( pos, surface);

	if(projection.NbPoints() > 0)
	{               
		projection.LowerDistanceParameters(*u, *v);
		return true;
	}     
	return false;
}

bool CFace::GetClosestPoint(const gp_Pnt &pos, gp_Pnt &closest_pnt)const{
	BRepPrimAPI_MakeBox cuboid(gp_Ax2(pos, gp_Vec(1, 0, 0), gp_Vec(0, 1, 0)), 0.0001, 0.0001, 0.0001);

	BRepExtrema_DistShapeShape extrema(m_topods_face, cuboid.Shape());
	if(extrema.Perform() != Standard_True)return false;
	closest_pnt = extrema.PointOnShape1(1);

	return true;
}

bool CFace::GetClosestSurfacePoint(const gp_Pnt &pos, gp_Pnt &closest_pnt)const{
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
	wxGetApp().Add(new_object, NULL);
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
		gp_Dir x_direction = plane.XAxis().Direction();
		gp_Dir y_direction = plane.YAxis().Direction();
		if(!face_for_tools->Face().Orientation())
		{
			// swap the axes to invert the normal
			y_direction = plane.XAxis().Direction();
			x_direction = plane.YAxis().Direction();
		}
		CoordinateSystem* new_object = new CoordinateSystem(_("Face Coordinate System"), plane.Location(), x_direction, y_direction);
		wxGetApp().Add(new_object, NULL);
		wxGetApp().m_marked_list->Clear(true);
		wxGetApp().m_marked_list->Add(new_object, true);
		wxGetApp().Repaint();
	}
};

static MakeCoordSystem make_coordsys;

class SketchOnFace:public Tool
{
	// only use this if GetSurfaceType() == GeomAbs_Plane
public:
	const wxChar* GetTitle(){return _("Sketch On Face");}
	wxString BitmapPath(){return _T("sketchmode");}
	void Run(){
		gp_Pln plane;
		face_for_tools->GetPlaneParams(plane);
		gp_Dir x_direction = plane.XAxis().Direction();
		gp_Dir y_direction = plane.YAxis().Direction();
		if(!face_for_tools->Face().Orientation())
		{
			// swap the axes to invert the normal
			y_direction = plane.XAxis().Direction();
			x_direction = plane.YAxis().Direction();
		}
		CoordinateSystem* coord_sys = new CoordinateSystem(_("Face Coordinate System"), plane.Location(), x_direction, y_direction);
		CSketch* sketch = new CSketch();
		sketch->Add(coord_sys,NULL);
		sketch->ReloadPointers();
		//TODO: should be faces solids parent
		wxGetApp().Add(sketch, NULL);
		wxGetApp().m_marked_list->Clear(true);
		wxGetApp().m_marked_list->Add(sketch, true);
		wxGetApp().EnterSketchMode(sketch);
		wxGetApp().Repaint();
		wxGetApp().m_frame->m_input_canvas->RefreshByRemovingAndAddingAll();
	}
};

static SketchOnFace sketch_on_face;

class ExtrudeFace:public Tool
{
public:
	const wxChar* GetTitle(){return _("Extrude Face");}
	wxString BitmapPath(){return _T("extface");}
	void Run(){
		wxGetApp().m_marked_list->Clear(false);
		wxGetApp().m_marked_list->Add(face_for_tools, false);
		PickCreateExtrusion();
	}
};

static ExtrudeFace extrude_face;

class RotateToFace:public Tool
{
public:
	const wxChar* GetTitle(){return _("Rotate to Face");}
	wxString BitmapPath(){return _T("rotface");}
	void Run(){
		gp_Pln plane;
		face_for_tools->GetPlaneParams(plane);
		gp_Dir x_direction = plane.XAxis().Direction();
		gp_Dir y_direction = plane.YAxis().Direction();
		if(face_for_tools->Face().Orientation()== TopAbs_REVERSED)
		{
			// swap the axes to invert the normal
			y_direction = plane.XAxis().Direction();
			x_direction = plane.YAxis().Direction();
		}
		gp_Trsf face_matrix = make_matrix(plane.Location(), x_direction, y_direction);
		gp_Trsf inv_matrix = face_matrix.Inverted();

		double m[16];
		extract(face_matrix.Inverted(), m);
		// if any objects are selected, move them
		if(wxGetApp().m_marked_list->list().size() > 0)
		{
			for(std::list<HeeksObj *>::iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
			{
				HeeksObj* object = *It;
				object->ModifyByMatrix(m);
			}
		}
		else
		{
			// move the solid
			HeeksObj* parent_body = face_for_tools->GetParentBody();
			if(parent_body)parent_body->ModifyByMatrix(m);
		}
	}
};

static RotateToFace rotate_to_face;

void CFace::GetTools(std::list<Tool*>* t_list, const wxPoint* p){
	face_for_tools = this;
	t_list->push_back(&make_sketch_tool);
	if(IsAPlane(NULL))
	{
		t_list->push_back(&make_coordsys);
		t_list->push_back(&sketch_on_face);
		t_list->push_back(&rotate_to_face);
	}
	t_list->push_back(&extrude_face);
}

void CFace::GetGripperPositionsTransformed(std::list<GripData> *list, bool just_for_endof)
{
	if(GetParentBody() == NULL)
	{
		HeeksObj::GetGripperPositionsTransformed(list, just_for_endof);
	}
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

bool CFace::IsAPlane(gp_Pln *returned_plane)
{
	static const int GRID = 5;
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	GeomAbs_SurfaceType surface_type = surface.GetType();
	switch(surface_type)
	{
	case GeomAbs_Plane:
		if(returned_plane)
		{
			BRepAdaptor_Surface surface(m_topods_face, Standard_True);
			*returned_plane = surface.Plane();
		}
		return true;
	case GeomAbs_Cylinder:
	case GeomAbs_Cone:
	case GeomAbs_Sphere:
	case GeomAbs_Torus:
		return false;
	default:
		{
			double uv_box[4];
			GetUVBox(uv_box);
			double u[GRID + 1];
			double v[GRID + 1];
			double U = uv_box[1] - uv_box[0];
			double V = uv_box[3] - uv_box[2];
			for(int i = 0; i <= GRID; i++)
			{
				u[i] = uv_box[0] + U * i / GRID;
				v[i] = uv_box[2] + V * i / GRID;
			}

			// define plane from three corners
			gp_Pnt p00, pN0, pNN;
			gp_Dir n00 = GetNormalAtUV(u[0], v[0], &p00);
			gp_Dir nN0 = GetNormalAtUV(u[GRID], v[0], &pN0);
			gp_Dir nNN = GetNormalAtUV(u[GRID], v[GRID], &pNN);

			gp_Trsf m;
			try
			{
				m = make_matrix(p00, make_vector(p00, pN0), make_vector(p00, pNN));
			}
			catch(...)
			{
				// matrix failed, probably two points in the same place. Try again 0.1 inwards from the edges
				gp_Dir n00 = GetNormalAtUV(uv_box[0] + U * 0.1, uv_box[2] + V * 0.1, &p00);
				gp_Dir nN0 = GetNormalAtUV(uv_box[0] + U * 0.9, uv_box[2] + V * 0.1, &pN0);
				gp_Dir nNN = GetNormalAtUV(uv_box[0] + U * 0.9, uv_box[2] + V * 0.9, &pNN);
				m = make_matrix(p00, make_vector(p00, pN0), make_vector(p00, pNN));
			}
			gp_Pln plane(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
			plane.Transform(m);

			// test all the vertices of the grid
			for(int i = 0; i <= GRID; i++)
			{
				for(int j = 0; j <= GRID; j++)
				{
					gp_Pnt p;
					gp_Dir n = GetNormalAtUV(u[i], v[j], &p);

					// check the point lies on the plane
					double d = fabs(plane.Distance(p));
					if(d > wxGetApp().m_geom_tol)
						return false;
				}
			}

			if(returned_plane)*returned_plane = plane;
			return true; // all points tested fitted the plane
		}
	}
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
	// returns a plane for the underlying surface
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	GeomAbs_SurfaceType surface_type = surface.GetType();
	if(surface_type == GeomAbs_Plane)
	{
		p = surface.Plane();
	}
	else
	{
		IsAPlane(&p);
		if(	m_topods_face.Orientation()== TopAbs_REVERSED )
		{
			p = gp_Pln(p.Axis().Location(), -p.Axis().Direction());
			
		}
	}
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

void CFace::GetTorusParams(gp_Torus &t)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	t = surface.Torus();
}

bool CFace::GetNurbSurfaceParams(CNurbSurfaceParams* params)
{
	try {
		BRepAdaptor_Surface surface(m_topods_face, Standard_True);
		params->rational = surface.IsURational() != 0;
		params->is_u_periodic = surface.IsUPeriodic() != 0;
		params->is_v_periodic = surface.IsVPeriodic() != 0;
		params->is_u_closed = surface.IsUClosed() != 0;
		params->is_v_closed = surface.IsVClosed() != 0;
		switch(surface.GetType())
		{
		case GeomAbs_BSplineSurface:
		case GeomAbs_BezierSurface:
		case GeomAbs_SurfaceOfExtrusion:
			params->u_order = surface.UDegree();
			params->v_order = surface.VDegree();
			params->n_u_vertices = surface.NbUPoles();
			params->n_v_vertices = surface.NbVPoles();
			break;
		default:
			params->u_order = 0;
			params->v_order = 0;
			params->n_u_vertices = 0;
			params->n_v_vertices = 0;
			break;
		}

		switch(surface.GetType())
		{
		case GeomAbs_BSplineSurface:
			{
				Handle(Geom_Surface) aGeomSurface = BRep_Tool::Surface(m_topods_face);
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
			break;

		case GeomAbs_BezierSurface:
			{
				Handle(Geom_Surface) aGeomSurface = BRep_Tool::Surface(m_topods_face);
				Handle(Geom_BezierSurface) aBSplineSurface = Handle(Geom_BezierSurface)::DownCast(aGeomSurface);

				int myNbUPoles = aBSplineSurface->NbUPoles();
				int myNbVPoles = aBSplineSurface->NbVPoles();

				TColgp_Array2OfPnt aPoles(1, myNbVPoles, 1, myNbUPoles);

				aBSplineSurface->Poles(aPoles);
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

			}
			break;
		default:
			break;
		}

	}
	catch(Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		wxMessageBox(wxString(_("Error in CFace::GetNurbSurfaceParams")) + _T(": ") + Ctt(e->GetMessageString()));
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

void CFace::GetSurfaceUVPeriod(double *uv, bool *isUPeriodic, bool *isVPeriodic)
{
	BRepAdaptor_Surface surface(m_topods_face, Standard_True);
	uv[0] = surface.UPeriod();
	uv[1] = surface.VPeriod();
	if(isUPeriodic)*isUPeriodic = (surface.IsUPeriodic() != Standard_False);
	if(isVPeriodic)*isVPeriodic = (surface.IsVPeriodic() != Standard_False);
}

CShape* CFace::GetParentBody()
{
	if(Owner() == NULL)return NULL;
	if(Owner()->Owner() == NULL)return NULL;
	if(Owner()->Owner()->GetType() != SolidType)return NULL;
	return (CShape*)(Owner()->Owner());
}

void CFace::MakeSureMarkingGLListExists()
{
	// create a marking display list
	if(!m_marking_gl_list)
	{
		CShape* parent_body = GetParentBody();
		if(parent_body)
		{
			m_marking_gl_list = glGenLists(1);
			glNewList(m_marking_gl_list, GL_COMPILE_AND_EXECUTE);
			glEndList();
		}
	}
}

void CFace::KillMarkingGLList()
{
	if (m_marking_gl_list)
	{
		glDeleteLists(m_marking_gl_list, 1);
		m_marking_gl_list = 0;
	}
}

void CFace::UpdateMarkingGLList(bool marked)
{
	if(m_marking_gl_list)
	{
		glNewList(m_marking_gl_list, GL_COMPILE);

		if(marked)
		{
			Material(wxGetApp().face_selection_color).glMaterial(1.0);
			glDisable(GL_BLEND);
			glDepthMask(1);
		}
		else
		{
			// use the parent body's colour
			CShape* parent_body = GetParentBody();
			if(parent_body)Material(parent_body->m_color).glMaterial(parent_body->GetOpacity());
			else
			{
				Material().glMaterial(1.0);
				glDisable(GL_BLEND);
				glDepthMask(1);
			}
		}

		glEndList();
	}
}
