// FaceTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "FaceTools.h"

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


void MeshFace(TopoDS_Face face, double pixels_per_mm)
{
	BRepTools::Clean(face);
	BRepMesh::Mesh(face, 1/pixels_per_mm);
}

void command_callback(const double* x, const double* n)
{
	glNormal3d(n[0],n[1],n[2]);
	glVertex3d(x[0],x[1],x[2]);
	glNormal3d(n[3],n[4],n[5]);
	glVertex3d(x[3],x[4],x[5]);
	glNormal3d(n[6],n[7],n[8]);
	glVertex3d(x[6],x[7],x[8]);
}

void DrawFaceWithCommands(TopoDS_Face face)
{
	glBegin(GL_TRIANGLES);
	DrawFace(face,command_callback,false);
	glEnd();
}

gp_Dir GetFaceNormalAtUV(const TopoDS_Face &face, double u, double v, gp_Pnt *pos){
	if(face.IsNull()) return gp_Dir(0, 0, 1);
	Handle(Geom_Surface) surf=BRep_Tool::Surface(face);          // get surface properties
	GeomLProp_SLProps props(surf, u, v, 1, 0.01);          // get surface normal
	if(!props.IsNormalDefined())return gp_Dir(0, 0, 1);
	gp_Dir norm=props.Normal();                         // check orientation
	if(pos)*pos = props.Value();
	if(face.Orientation()==TopAbs_REVERSED) norm.Reverse();
	return norm;
}

void DrawFace(TopoDS_Face face,void(*callbackfunc)(const double* x, const double* n), bool just_one_average_normal)
{
	double x[9], n[9];

	StdPrs_ToolShadedShape SST;

	// Get triangulation
	TopLoc_Location L;
	Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(face,L);
	gp_Trsf tr = L;

	if(facing.IsNull()){
#if 0
		BRepAdaptor_Surface surface(face, Standard_True);
		double u0 = surface.FirstUParameter();
		double u1 = surface.LastUParameter();
		double v0 = surface.FirstVParameter();
		double v1 = surface.LastVParameter();

		const int numi = 10;
		const int numj = 10;

		for(int i = 0; i<numi; i++)
		{
			for(int j = 0; j<numj; j++)
			{
				double uA = -1.2 + 2.5 *(double)i / numi;
				double uB = -1.2 + 2.5 *(double)(i+1) / numi;
				double vA = 10* (double)j / numj;
				double vB = 10*(double)(j+1) / numj;
				gp_Pnt p0, p1, p2, p3;
				gp_Dir n0 = GetFaceNormalAtUV(face, uA, vA, &p0);
				gp_Dir n1 = GetFaceNormalAtUV(face, uB, vA, &p1);
				gp_Dir n2 = GetFaceNormalAtUV(face, uB, vB, &p2);
				gp_Dir n3 = GetFaceNormalAtUV(face, uA, vB, &p3);

				x[0] = p0.X();
				x[1] = p0.Y();
				x[2] = p0.Z();
				x[3] = p1.X();
				x[4] = p1.Y();
				x[5] = p1.Z();
				x[6] = p2.X();
				x[7] = p2.Y();
				x[8] = p2.Z();

				n[0] = n0.X();
				n[1] = n0.Y();
				n[2] = n0.Z();
				n[3] = n1.X();
				n[4] = n1.Y();
				n[5] = n1.Z();
				n[6] = n2.X();
				n[7] = n2.Y();
				n[8] = n2.Z();

				(*callbackfunc)(x, n);

				x[0] = p0.X();
				x[1] = p0.Y();
				x[2] = p0.Z();
				x[3] = p2.X();
				x[4] = p2.Y();
				x[5] = p2.Z();
				x[6] = p3.X();
				x[7] = p3.Y();
				x[8] = p3.Z();

				n[0] = n0.X();
				n[1] = n0.Y();
				n[2] = n0.Z();
				n[3] = n2.X();
				n[4] = n2.Y();
				n[5] = n2.Z();
				n[6] = n3.X();
				n[7] = n3.Y();
				n[8] = n3.Z();

				(*callbackfunc)(x, n);
			}
		}
#endif
	}
	else
	{
		Poly_Connect pc(facing);	
		const TColgp_Array1OfPnt& Nodes = facing->Nodes();
		const Poly_Array1OfTriangle& triangles = facing->Triangles();
		TColgp_Array1OfDir myNormal(Nodes.Lower(), Nodes.Upper());

		SST.Normal(face, pc, myNormal);
		double Umin, Umax, Vmin, Vmax;
		BRepTools::UVBounds(face,Umin, Umax, Vmin, Vmax);

		Standard_Integer nnn = facing->NbTriangles();					// nnn : nombre de triangles
		Standard_Integer nt, n1, n2, n3 = 0;						// nt  : triangle courant
		// ni  : sommet i du triangle courant
		for (nt = 1; nt <= nnn; nt++)					
		{
			if (SST.Orientation(face) == TopAbs_REVERSED)			// si la face est "reversed"
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

