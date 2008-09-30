// Face.cpp
#include "stdafx.h"
#include "Face.h"
#include "Shape.h"
#include <BRepMesh.hxx>
#include <StdPrs_ToolShadedShape.hxx>
#include <Poly_Connect.hxx>
#include <Poly_Triangulation.hxx>
#include <TColgp_Array1OfDir.hxx>
#include <BRepTools.hxx>
#include <GeomLProp_SLProps.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include "../interface/Tool.h"
#include <TopTools_ListIteratorOfListOfShape.hxx>

wxIcon* CFace::m_icon = NULL;

CFace::CFace(const TopoDS_Face &face):m_topods_face(face){

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
	m_material.glMaterial(1.0);
	
	if(m_owner && m_owner->m_owner && m_owner->m_owner->GetType() == SolidType) {
		// using existing BRepMesh::Mesh
	}
	else {
		double pixels_per_mm = wxGetApp().GetPixelScale();
		BRepTools::Clean(m_topods_face);
		BRepMesh::Mesh(m_topods_face, 1/pixels_per_mm);
	}

	{
		StdPrs_ToolShadedShape SST;

		// Get triangulation
		TopLoc_Location L;
		Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(m_topods_face,L);
		gp_Trsf tr = L;

		if(!facing.IsNull()){
			glEnable(GL_LIGHTING);
			glShadeModel(GL_SMOOTH);
			glBegin(GL_TRIANGLES);
			Poly_Connect pc(facing);	
			const TColgp_Array1OfPnt& Nodes = facing->Nodes();
			const TColgp_Array1OfPnt2d& UVNodes = facing->UVNodes();
			const Poly_Array1OfTriangle& triangles = facing->Triangles();
			TColgp_Array1OfDir myNormal(Nodes.Lower(), Nodes.Upper());

			SST.Normal(m_topods_face, pc, myNormal);
			double Umin, Umax, Vmin, Vmax;
			BRepTools::UVBounds(m_topods_face,Umin, Umax, Vmin, Vmax);
			double dUmax = (Umax - Umin);
			double dVmax = (Vmax - Vmin);

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

					glNormal3d(myNormal(n1).X(), myNormal(n1).Y(), myNormal(n1).Z());
					glVertex3d(v1.X(), v1.Y(), v1.Z());
					glNormal3d(myNormal(n2).X(), myNormal(n2).Y(), myNormal(n2).Z());
					glVertex3d(v2.X(), v2.Y(), v2.Z());
					glNormal3d(myNormal(n3).X(), myNormal(n3).Y(), myNormal(n3).Z());
					glVertex3d(v3.X(), v3.Y(), v3.Z());
				}
			}
			glEnd();
			glDisable(GL_LIGHTING);
			glShadeModel(GL_FLAT);
		}
	}
}

void CFace::GetBox(CBox &box){
	if(!m_box.m_valid)
	{
		// there must be a better way than re-using the render code
		// Get triangulation
		if(m_owner && m_owner->m_owner && m_owner->m_owner->GetType() != SolidType){
			double pixels_per_mm = wxGetApp().GetPixelScale();
			BRepTools::Clean(m_topods_face);
			BRepMesh::Mesh(m_topods_face, 1/pixels_per_mm);
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

wxIcon* CFace::GetIcon(){
	if(m_icon == NULL)
	{
		wxString exe_folder = wxGetApp().GetExeFolder();
		m_icon = new wxIcon(exe_folder + "/icons/face.png", wxBITMAP_TYPE_PNG);
	}
	return m_icon;
}

class OffsetFaceTool:public Tool{
	CFace* m_face;
public:
	OffsetFaceTool(CFace* face):m_face(face){}

	// Tool's virtual functions
	void Run(){
		BRepOffsetAPI_MakeOffset make_operation(m_face->Face());
		make_operation.Perform(-6.0);
		HeeksObj* new_object = CShape::MakeObject(make_operation.Shape(), "Result of Face Offset");
		if(make_operation.Generated(m_face->Face()).Extent() > 0){
			wxMessageBox("Generated");
		}
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().DeleteUndoably(m_face);
	}
	const char* GetTitle(){ return "Offset Face";}
};

void CFace::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	t_list->push_back(new OffsetFaceTool(this));
}

void CFace::ModifyByMatrix(const double *m){
	gp_Trsf mat = make_matrix(m);
	BRepBuilderAPI_Transform myBRepTransformation(m_topods_face,mat);
	TopoDS_Shape new_shape = myBRepTransformation.Shape();
	wxGetApp().Add(new CFace(*((TopoDS_Face*)(&new_shape))), NULL);
	wxGetApp().DeleteUndoably(this);
}

void CFace::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal){
	BRepTools::Clean(m_topods_face);
	BRepMesh::Mesh(m_topods_face, cusp);

	StdPrs_ToolShadedShape SST;

	// Get triangulation
	TopLoc_Location L;
	Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(m_topods_face,L);
	gp_Trsf tr = L;

	if(!facing.IsNull()){
		Poly_Connect pc(facing);	
		const TColgp_Array1OfPnt& Nodes = facing->Nodes();
		const TColgp_Array1OfPnt2d& UVNodes = facing->UVNodes();
		const Poly_Array1OfTriangle& triangles = facing->Triangles();
		TColgp_Array1OfDir myNormal(Nodes.Lower(), Nodes.Upper());

		SST.Normal(m_topods_face, pc, myNormal);
		double Umin, Umax, Vmin, Vmax;
		BRepTools::UVBounds(m_topods_face,Umin, Umax, Vmin, Vmax);
		double dUmax = (Umax - Umin);
		double dVmax = (Vmax - Vmin);

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
	BRepTools::UVBounds(m_topods_face, umin, umax, vmin, vmax);          // create surface
	Handle(Geom_Surface) surf=BRep_Tool::Surface(m_topods_face);          // get surface properties
	GeomLProp_SLProps props(surf, (umin + umax)/2, (vmin + vmax)/2, 1, 0.01);          // get surface normal
	gp_Dir norm=props.Normal();                         // check orientation
	if(pos)*pos = props.Value();
	if(m_topods_face.Orientation()==TopAbs_REVERSED) norm.Reverse();
	return norm;
}

double CFace::Area()const{
	GProp_GProps System;
	BRepGProp::SurfaceProperties(m_topods_face,System);
	return System.Mass();
}

void CFace::WriteXML(TiXmlElement *root)
{
	CShape::m_solids_found = true;
}
