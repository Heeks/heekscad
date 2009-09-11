// FaceTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "FaceTools.h"

void MeshFace(TopoDS_Face face, double pixels_per_mm)
{
	BRepTools::Clean(face);
	BRepMesh::Mesh(face, 1/pixels_per_mm);
}

void DrawFace(TopoDS_Face face)
{
	glBegin(GL_TRIANGLES);

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

			if (true)//TriangleIsValid (Nodes(n1),Nodes(n2),Nodes(n3)) )
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
	glEnd();
}

