// RuledSurface.h

extern void PickCreateRuledSurface();
extern void PickCreateExtrusion();
extern bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape);
extern bool CreateExtrusion(const std::list<TopoDS_Shape> &wire_or_face_list, TopoDS_Shape& shape);
