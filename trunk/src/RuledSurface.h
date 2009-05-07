// RuledSurface.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

extern void PickCreateRuledSurface();
extern void PickCreateExtrusion();
extern bool ConvertLineArcsToWire2(const std::list<HeeksObj*> &list, TopoDS_Wire &wire);
extern HeeksObj* CreateExtrusion(std::list<HeeksObj*> list, double height);
extern HeeksObj* CreatePipeFromProfile(HeeksObj* spine, HeeksObj* profile);
extern bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape);
extern void CreateExtrusions(const std::list<TopoDS_Face> &faces, std::list<TopoDS_Shape>& new_shapes, const gp_Vec& extrude_vector);
