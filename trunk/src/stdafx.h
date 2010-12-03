// stdafx.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#if wxUSE_UNICODE
	#ifndef _UNICODE
		#define _UNICODE
	#endif
#endif

#define _WARNINGS 3

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#ifdef WIN32
#pragma warning(disable:4100)
#pragma warning(  disable : 4244 )        // Issue warning 4244
#endif

#ifdef WIN32
#pragma warning(  default : 4244 )        // Issue warning 4244
#endif

#ifdef WIN32
#include "windows.h"
#endif

#include <wx/wx.h>
#if wxUSE_UNICODE
	#ifndef _UNICODE
		#define _UNICODE
	#endif
#endif

#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <ctime>
#include <iostream>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Following is required to be defined on Ubuntu with OCC 6.3.1
#ifndef HAVE_IOSTREAM
#define HAVE_IOSTREAM
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef INT_MAX
	#define INT_MAX 2147483647
#endif
#ifndef INT_MIN
	#define INT_MIN  (-2147483647 - 1)
#endif



#include <Standard.hxx>
#include <Standard_TypeDef.hxx>

#include <Bnd_Box.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgo_Fuse.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <BRepGProp.hxx>
#include <BRepMesh.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRepOffsetAPI_MakeEvolved.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_Sewing.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GC_MakeSegment.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Geom_Axis1Placement.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <GeomAPI_IntCS.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomAPI_IntSS.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GeomLProp_SLProps.hxx>
#include <GProp_GProps.hxx>
#include <gp.hxx>
#include <gp_Circ.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Elips.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Handle_Geom_TrimmedCurve.hxx>
#include <IGESControl_Controller.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <IntTools_FaceFace.hxx>
#include "math_BFGS.hxx"
#include "math_MultipleVarFunctionWithGradient.hxx"
#include <Poly_Connect.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <ShapeFix_Wire.hxx>
#include <Standard_ErrorHandler.hxx>
#include <StdPrs_ToolShadedShape.hxx>
#include <STEPControl_Controller.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <TColgp_Array1OfDir.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopOpeBRep_FacesIntersector.hxx>
#include <TopOpeBRepBuild_FuseFace.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapIteratorOfMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <UnitsAPI.hxx>

#include <wx/aui/aui.h>
#include "wx/brush.h"
#include "wx/button.h"
#include <wx/clipbrd.h>
#include <wx/checklst.h>
#include "wx/choice.h"
#include <wx/choicdlg.h>
#include <wx/cmdline.h>
#include "wx/combobox.h"
#include <wx/confbase.h>
#include <wx/config.h>
#include "wx/cursor.h"
#include <wx/dc.h>
#include "wx/dcclient.h"
#include <wx/dcmirror.h>
#include "wx/defs.h"
#include "wx/dirdlg.h"
#include "wx/dnd.h"
#include <wx/dynlib.h>
#include "wx/event.h"
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/glcanvas.h>
#include "wx/hash.h"
#include <wx/image.h>
#include <wx/imaglist.h>
#include "wx/intl.h"
#include "wx/layout.h"
#include "wx/log.h"
#include <wx/menuitem.h>
#include "wx/msgdlg.h"
#include "wx/object.h"
#include "wx/panel.h"
#include "wx/pen.h"
#include "wx/popupwin.h"
#include <wx/print.h>
#include <wx/printdlg.h>
#include "wx/settings.h"
#include "wx/scrolwin.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include <wx/stdpaths.h>
#include "wx/string.h"
#include <wx/sizer.h>
#include "wx/textctrl.h"
#include "wx/textdlg.h"
#include <wx/toolbar.h>
#include <wx/tooltip.h>
#include <wx/treectrl.h>
#include "wx/window.h"


#include "../tinyxml/tinyxml.h"

extern "C" {
#include <GL/gl.h>
#include <GL/glu.h>
}

//#define USE_UNDO_ENGINE


#define CHECK_FOR_INVALID_CONSTRAINT//JT This is my attempt to isolate

#ifdef CHECK_FOR_INVALID_CONSTRAINT //
// THESE ARE SOME OPTIONS ON HOW TO HANDLE THIS STUFF
//#define DISPLAY_CHECK_FOR_INVALID_CONSTRAINT_ERROR_MSGBOX
//#define LET_BAD_CONSTRAINT_PASS //JT Sometimes you know things are hosed up and you want to see what happens

#endif

//JT THIS IS ATTEMPT TO CREATE A TEST PROBE FUNCTION THAT CAN BE USED TO IDENTIFY ROOT CAUSE OF CONSTRAINT ERROR
//#define CONSTRAINT_TESTER

#ifdef CONSTRAINT_TESTER

 #define FIRE_CONSTRAINT_TESTER_FROM_MAIN_MENU
#endif








#include "../interface/strconv.h"
#include "../interface/HeeksObj.h"
#include "../interface/HeeksColor.h"
#include "../interface/Material.h"
#include "../interface/InputMode.h"
#include "../interface/Tool.h"
#include "../interface/PropertyString.h"
#include "../interface/ObjectCanvas.h"
#include "HeeksCAD.h"

#include "ConversionTools.h"
#include "CoordinateSystem.h"
#include "Face.h"
#include "Edge.h"
#include "Loop.h"
#include "Gripper.h"
#include "Geom.h"
//#include "Loop.h"
#include "MarkedList.h"
#include "Shape.h"
#include "Sketch.h"
#include "Solid.h"

// Visual Studio 2010 work arround

#if _MSC_VER == 1600
	#include <iterator>
#endif
