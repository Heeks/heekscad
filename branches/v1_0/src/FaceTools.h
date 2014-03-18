// FaceTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

void MeshFace(TopoDS_Face face, double pixels_per_mm);
void DrawFace(TopoDS_Face face,void(*callbackfunc)(const double* x, const double* n), bool just_one_average_normal);
void DrawFaceWithCommands(TopoDS_Face face);
gp_Dir GetFaceNormalAtUV(const TopoDS_Face &face, double u, double v, gp_Pnt *pos);

