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

void DrawFace(TopoDS_Face face,void(*callbackfunc)(const double* x, const double* n), bool just_one_average_normal)
{
	double x[9], n[9];

	StdPrs_ToolShadedShape SST;

	// Get triangulation
	TopLoc_Location L;
	Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(face,L);
	gp_Trsf tr = L;

	if(!facing.IsNull()){
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

