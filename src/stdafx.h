// stdafx.h
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

#include <Standard_ErrorHandler.hxx>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>
#include <gp_Cylinder.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <Geom_Axis1Placement.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <Geom_BezierCurve.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepOffsetAPI_MakeEvolved.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp.hxx>
#include <Geom_Plane.hxx>
#include <BRep_Tool.hxx>
#include <Precision.hxx>

#include <UnitsAPI.hxx>

#include "Geom.h"

extern "C" {
#include <GL/gl.h>
#ifdef WIN32
#include <GL/glu.h>
#else
#include <GL/glu.h>
#endif
}

#include "HeeksCAD.h"

#include "../interface/strconv.h"

