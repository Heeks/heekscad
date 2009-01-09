// RuledSurface.h

#include "TopoDS_Wire.hxx"
#include "TopoDS_Face.hxx"

class gp_Vec;

extern void PickCreateRuledSurface();
extern void PickCreateExtrusion();
extern bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape);
extern void CreateExtrusions(const std::list<TopoDS_Face> &faces, std::list<TopoDS_Shape>& new_shapes, const gp_Vec& extrude_vector);
