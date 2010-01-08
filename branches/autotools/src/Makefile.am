AM_CXXFLAGS =\
	 -Wall -g -I "./" -I /usr/include/opencascade  -I ${CASINCPATH} $(GLINCLUDES) $(FLAGS_64BIT) -DUSE_GENERIC_TREECTRL -DUNICODE -DHEEKSCAD -DHAVE_LIMITS -DWXUSINGDLL -DHAVE_LIMITS -DTIXML_USE_STL ${RIP} $(PYTHONCFLAGS)  `wx-config  --cflags`



AM_LDFLAGS=-pthread -Wl,-Bsymbolic-functions -lwx_gtk2u_richtext-2.8 -lwx_gtk2u_aui-2.8 -lwx_gtk2u_xrc-2.8 -lwx_gtk2u_qa-2.8 -lwx_gtk2u_html-2.8 -lwx_gtk2u_adv-2.8 -lwx_gtk2u_core-2.8 -lwx_baseu_xml-2.8 -lwx_baseu_net-2.8 -lwx_baseu-2.8 -lstdc++ -lGLU ${PYTHONLIBS} -lGL -g ${OCCLIBS} `wx-config --libs --gl-libs`

#`wx-config  --libs` 

#-lTKVRML -lTKSTL -lTKBRep -lTKIGES -lTKShHealing -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKXSBase -lTKShapeSchema -lFWOSPlugin -lTKBool -lTKCAF -lTKCDF -lTKernel -lTKFeat -lTKFillet -lTKG2d -lTKG3d -lTKGeomAlgo -lTKGeomBase -lTKHLR -lTKMath -lTKOffset -lTKPrim -lTKPShape -lTKService -lTKTopAlgo -lTKV2d -lTKV3d -lTKMesh -lTKAdvTools -lTKBO -lTKXDESTEP -lTKXCAF -lTKXCAFSchema -lTKLCAF -lTKPLCAF ${CASLIBPATH}

# ifeq ($(strip $(PREFIX)),)
#   PREFIX=/usr/local
# endif
# BINDIR=$(PREFIX)/bin
# DATADIR=$(PREFIX)/share
# HEEKSCADDIR=$(DATADIR)/heekscad
# PIXMAPSDIR=$(DATADIR)/pixmaps
# APPSDIR=$(DATADIR)/applications
# mkdir_p= mkdir -p
# CC = g++

# ifeq ($(strip $(PYTHONCFLAGS)),)
#   PYTHONCFLAGS=$(shell python-config --includes)
# endif

# #run-in-place, for people on linux who don't want to sudo make install:
# ifeq ($(strip $(RUNINPLACE)),yes)
#   RIP=-DRUNINPLACE
# else
#   RIP= 
# endif

# FLAGS_64BIT=$(shell if [ x`getconf LONG_BIT` = x64 ] ; then echo "-D_OCC64 -m64" ; fi )

# ifneq ($(strip $(CASROOT)),)
#     CASINCPATH=$(CASROOT)/../inc
# else
#     CASINCPATH=/usr/include/opencascade
# endif



bin_PROGRAMS = HeeksCAD

#EXECUTABLE=../
#                 translations/Makefile
#                 unittest/Makefile
HeeksCAD_SOURCES=AboutBox.cpp advprops.cpp BezierCurve.cpp Cone.cpp ConversionTools.cpp CoordinateSystem.cpp Cuboid.cpp Cylinder.cpp DigitizedPoint.cpp DigitizeMode.cpp DimensionDrawing.cpp Drawing.cpp dxf.cpp svg.cpp Edge.cpp Face.cpp Geom.cpp glfont2.cpp GLList.cpp GraphicsCanvas.cpp Grid.cpp Gripper.cpp GripperSelTransform.cpp Group.cpp HArc.cpp HCircle.cpp HDimension.cpp HeeksCAD.cpp HeeksFrame.cpp HeeksPrintout.cpp HILine.cpp HEllipse.cpp HImage.cpp HLine.cpp HPoint.cpp HSpline.cpp HText.cpp HXml.cpp InputModeCanvas.cpp LineArcDrawing.cpp Loop.cpp MagDragWindow.cpp manager.cpp MarkedList.cpp ObjPropsCanvas.cpp odcombo.cpp OptionsCanvas.cpp Plugins.cpp PointDrawing.cpp PointOrWindow.cpp PropertiesCanvas.cpp PropertyTrsf.cpp ../interface/PropertyVertex.cpp propgrid.cpp props.cpp RegularShapesDrawing.cpp RuledSurface.cpp Ruler.cpp SelectMode.cpp Shape.cpp ShapeData.cpp Sketch.cpp Solid.cpp Sphere.cpp StlSolid.cpp StretchTool.cpp TransformTool.cpp TransformTools.cpp TreeCanvas.cpp UndoEngine.cpp Vertex.cpp ViewPoint.cpp ViewRotating.cpp ViewZooming.cpp Window.cpp Wire.cpp wxImageLoader.cpp ../interface/HeeksCADInterface.cpp ../interface/HeeksColor.cpp ../interface/HeeksObj.cpp ../interface/DoubleInput.cpp ../interface/LeftAndRight.cpp ../interface/MarkedObject.cpp ../interface/ObjList.cpp ../interface/PropertyCheck.cpp ../interface/PropertyChoice.cpp ../interface/PropertyColor.cpp ../interface/PropertyDouble.cpp ../interface/PropertyLength.cpp ../interface/PropertyInt.cpp ../interface/PropertyList.cpp ../interface/PropertyString.cpp ../interface/strconv.cpp ../interface/ToolImage.cpp ../interface/ToolList.cpp ../interface/TransientObject.cpp ../tinyxml/tinyxml.cpp ../tinyxml/tinyxmlerror.cpp ../tinyxml/tinyxmlparser.cpp Constraint.cpp ConstrainedObject.cpp ConstraintTools.cpp EndedObject.cpp SolveSketch.cpp ../sketchsolve/src/solve.cpp CorrelationTool.cpp sketchinterface.cpp ../sketchsolve/src/solveimpl.cpp ../sketchsolve/src/errorfuncs.cpp OCCSolver.cpp MultiPoly.cpp BentleyOttmann.cpp SimpleIntersector.cpp NearMap.cpp Polygon.cpp RS274X.cpp FaceTools.cpp Pad.cpp FastCurves.cpp WrappedCurves.cpp SketchTools.cpp CxfFont.cpp DynamicSolid.cpp ShapeTools.cpp Part.cpp Pocket.cpp HAngularDimension.cpp AutoSave.cpp

nodist_HeeksCAD_SOURCES=AboutBox.h Geom.h                 Loop.h                  ShapeData.h advprops.h           glfont2.h              MagDragWindow.h         Shape.h AutoSave.h           GLList.h               manager.h               ShapeTools.h BentleyOttmann.h     GraphicsCanvas.h       MarkedList.h            SimpleIntersector.h BezierCurve.h        Grid.h                 matrix.h                Sketch.h Cone.h               Gripper.h              MultiPoly.h             SketchTools.h ConstrainedObject.h  GripperSelTransform.h  NearMap.h               Solid.h Constraint.h         Group.h                ObjPropsCanvas.h        SolveSketch.h ConstraintTools.h    HAngularDimension.h    OCCSolver.h             Sphere.h ConversionTools.h    HArc.h                 odcombo.h               stdafx.h CoordinateSystem.h   HCircle.h              OptionsCanvas.h         StlSolid.h CorrelationTool.h    HDimension.h           Pad.h                   StretchTool.h Cuboid.h             HeeksCAD.h             Part.h                  svg.h CxfFont.h            HeeksConfig.h          Plugins.h               TransformTool.h Cylinder.h           HeeksFrame.h           Pocket.h                TransformTools.h DigitizedPoint.h     HeeksPrintout.h        PointDrawing.h          TreeCanvas.h DigitizeMode.h       HEllipse.h             PointOrWindow.h         UndoEngine.h DimensionDrawing.h   HILine.h               Polygon.h               Vertex.h Drawing.h            HImage.h               propdev.h               ViewPoint.h dxf.h                HLine.h                PropertiesCanvas.h      ViewRotating.h DynamicSolid.h       HPoint.h               PropertyTrsf.h          ViewZooming.h EdgeBlend.h          HSpline.h              propgrid.h              Window.h Edge.h               HText.h                RegularShapesDrawing.h  Wire.h EndedObject.h        HXml.h                 RS274X.h                WrappedCurves.h Face.h               InputModeCanvas.h      RuledSurface.h          wxImageLoader.h  FaceTools.h          Intersector.h          Ruler.h FastCurves.h         LineArcDrawing.h       SelectMode.h










