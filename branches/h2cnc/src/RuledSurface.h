// RuledSurface.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

extern void PickCreateRuledSurface();
extern void PickCreateExtrusion();
extern void PickCreateRevolution();
extern void PickCreateSweep();
extern bool ConvertLineArcsToWire2(const std::list<HeeksObj*> &list, TopoDS_Wire &wire);
extern HeeksObj* CreateExtrusionOrRevolution(std::list<HeeksObj*> list, double height_or_angle, bool solid_if_possible, bool revolution_not_extrusion, double taper_angle_for_extrusion, bool add_new_objects = true);
extern HeeksObj* CreatePipeFromProfile(HeeksObj* spine, HeeksObj* profile);
extern HeeksObj* CreateRuledFromSketches(std::list<HeeksObj*> list, bool make_solid);
extern bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape, bool make_solid);
extern void CreateExtrusions(const std::list<TopoDS_Shape> &faces_or_wires, std::list<TopoDS_Shape>& new_shapes, const gp_Vec& extrude_vector, double taper_angle, bool solid_if_possible);
extern void CreateRevolutions(const std::list<TopoDS_Shape> &faces_or_wires, std::list<TopoDS_Shape>& new_shapes, const gp_Ax1& axis, double angle);
