// stdafx.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#include <list>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <iomanip>

#include <wx/wx.h>

#ifdef WIN32
#pragma warning(disable:4100)
#pragma warning(  disable : 4244 )        // Issue warning 4244
#endif

#ifdef WIN32
#pragma warning(  default : 4244 )        // Issue warning 4244
#endif

#include <Standard.hxx>

#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <BRepMesh.hxx>
#include <BRepOffsetAPI_MakeEvolved.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <Geom_Axis1Placement.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_Plane.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GeomLProp_SLProps.hxx>
#include <GProp_GProps.hxx>
#include <gp.hxx>
#include <gp_Circ.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <Poly_Connect.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <Standard_ErrorHandler.hxx>
#include <StdPrs_ToolShadedShape.hxx>
#include <TColgp_Array1OfDir.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <UnitsAPI.hxx>


extern "C" {
#include <GL/gl.h>
#include <GL/glu.h>
}

#include "../interface/strconv.h"
#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include "../interface/Material.h"
#include "../interface/InputMode.h"
#include "../interface/Tool.h"
#include "../interface/PropertyString.h"
#include "HeeksCAD.h"

#include "ConversionTools.h"
#include "CoordinateSystem.h"
#include "Face.h"
#include "Edge.h"
#include "Loop.h"
#include "Gripper.h"
#include "Geom.h"
#include "Loop.h"
#include "MarkedList.h"
#include "Shape.h"
#include "Sketch.h"
#include "Solid.h"

