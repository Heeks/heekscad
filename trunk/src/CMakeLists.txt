project(HeeksCAD)
cmake_minimum_required( VERSION 2.6 )

if( NOT DEFINED BUILD_STARTED_IN_RIGHT_DIR )
  message( FATAL_ERROR "Don't start a build in the src directory. Do a 'cd ..', then run cmake ." )
endif( NOT DEFINED BUILD_STARTED_IN_RIGHT_DIR )

set ( CMAKE_BUILD_TYPE Debug )
add_definitions ( -Wall -DUSE_GENERIC_TREECTRL -DHEEKSCAD -DUNICODE -DTIXML_USE_STL )
find_package( OpenCASCADE REQUIRED COMPONENTS TKVRML TKSTL TKBRep TKIGES TKShHealing TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKXSBase TKShapeSchema FWOSPlugin TKBool TKCAF TKCDF TKernel TKFeat TKFillet TKG2d TKG3d TKGeomAlgo TKGeomBase TKHLR TKMath TKOffset TKPrim TKPShape TKService TKTopAlgo TKV2d TKV3d TKMesh TKAdvTools TKBO TKXDESTEP TKXCAF TKXCAFSchema TKLCAF TKPLCAF )
FIND_PACKAGE( wxWidgets REQUIRED COMPONENTS base core gl aui )
find_package( PythonLibs REQUIRED )
INCLUDE(${wxWidgets_USE_FILE})

if( APPLE )
  # Apple includes two different openGL implementations, for Carbon and X11.  We want X11
  set( OSX_INCLUDE_DIRS /usr/X11R6/include )
  set( OSX_LIBS -L/usr/X11R6/lib -lGL -lX11 -lGLU -lglut )
endif( APPLE )

include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${wxWidgets_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS} . ${OSX_INCLUDE_DIRS}
    )
set ( EXECUTABLE_OUTPUT_PATH ../bin )

if( UNIX )
  #set( platform_SRCS some_necessary_linux_file.cpp )
else( UNIX )
  #windows
  #set( platform_SRCS dllmain.cpp stdafx.cpp )
endif( UNIX )

set( heekscad_HDRS
        AboutBox.h           glfont2.h              Loop.h                  ShapeData.h
        advprops.h           GLList.h               MagDragWindow.h         Shape.h
        AutoSave.h           GraphicsCanvas.h       manager.h               ShapeTools.h
        BentleyOttmann.h     Grid.h                 MarkedList.h            SimpleIntersector.h
        BezierCurve.h        Gripper.h              matrix.h                Sketch.h
        Cone.h               GripperSelTransform.h  MultiPoly.h             SketchTools.h
        ConstrainedObject.h  Group.h                NearMap.h               Solid.h
        Constraint.h         HAngularDimension.h    ObjPropsCanvas.h        SolidTools.h
        ConstraintTools.h    HArc.h                 OCCSolver.h             SolveSketch.h
        ConversionTools.h    HCircle.h              odcombo.h               Sphere.h
        CoordinateSystem.h   HDimension.h           OptionsCanvas.h         stdafx.h
        CorrelationTool.h    HDxf.h                 OrientationModifier.h   StlSolid.h
        Cuboid.h             HeeksCAD.h             Pad.h                   StretchTool.h
        CxfFont.h            HeeksConfig.h          Part.h                  svg.h
        Cylinder.h           HeeksFrame.h           Plugins.h               TransformTool.h
        DigitizedPoint.h     HeeksPrintout.h        Pocket.h                TransformTools.h
        DigitizeMode.h       HEllipse.h             PointDrawing.h          TreeCanvas.h
        DimensionDrawing.h   HILine.h               PointOrWindow.h         UndoEngine.h
        Drawing.h            HImage.h               Polygon.h               Vertex.h
        dxf.h                HLine.h                propdev.h               ViewPanning.h
        DynamicSolid.h       PropertiesCanvas.h     ViewPoint.h
        EdgeBlend.h          HPoint.h               PropertyTrsf.h          ViewRotating.h
        Edge.h               HSpline.h              propgrid.h              ViewZooming.h
        EndedObject.h        HText.h                RegularShapesDrawing.h  Window.h
        Face.h               HXml.h                 RS274X.h                Wire.h
        FaceTools.h          InputModeCanvas.h      RuledSurface.h          WrappedCurves.h
        FastCurves.h         Intersector.h          Ruler.h                 wxImageLoader.h
        Geom.h               LineArcDrawing.h       SelectMode.h
   )#Hole.h

set( heekscad_SRCS
        AboutBox.cpp           GLList.cpp               MarkedList.cpp            ShapeTools.cpp
        advprops.cpp           GraphicsCanvas.cpp       matrix.cpp                SimpleIntersector.cpp
        AutoSave.cpp           Grid.cpp                 MatrixFactorization.cpp   Sketch.cpp
        BentleyOttmann.cpp     Gripper.cpp              MultiPoly.cpp             sketchinterface.cpp
        BezierCurve.cpp        GripperSelTransform.cpp  NearMap.cpp               SketchTools.cpp
        Cone.cpp               Group.cpp                ObjPropsCanvas.cpp        Solid.cpp
        ConstrainedObject.cpp  HAngularDimension.cpp    OCCSolver.cpp             SolidTools.cpp
        Constraint.cpp         HArc.cpp                 odcombo.cpp               SolveSketch.cpp
        ConstraintTools.cpp    HCircle.cpp              OptionsCanvas.cpp         Sphere.cpp
        ConversionTools.cpp    HDimension.cpp           OrientationModifier.cpp   stdafx.cpp
        CoordinateSystem.cpp   HDxf.cpp                 Pad.cpp                   StlSolid.cpp
        CorrelationTool.cpp    HeeksCAD.cpp             Part.cpp                  StretchTool.cpp
        Cuboid.cpp             HeeksFrame.cpp           Plugins.cpp               svg.cpp
        CxfFont.cpp            HeeksPrintout.cpp        Pocket.cpp                TransformTool.cpp
        Cylinder.cpp           HEllipse.cpp             PointDrawing.cpp          TransformTools.cpp
        DigitizedPoint.cpp     HILine.cpp               PointOrWindow.cpp         TreeCanvas.cpp
        DigitizeMode.cpp       HImage.cpp               Polygon.cpp               UndoEngine.cpp
        DimensionDrawing.cpp   HLine.cpp                PropertiesCanvas.cpp      Vertex.cpp
        Drawing.cpp            PropertyTrsf.cpp         ViewPanning.cpp
        dxf.cpp                HPoint.cpp               propgrid.cpp              ViewPoint.cpp
        DynamicSolid.cpp       HSpline.cpp              props.cpp                 ViewRotating.cpp
        Edge.cpp               HText.cpp                RegularShapesDrawing.cpp  ViewZooming.cpp
        EndedObject.cpp        HXml.cpp                 RS274X.cpp                Window.cpp
        Face.cpp               InputModeCanvas.cpp      RuledSurface.cpp          Wire.cpp
        FaceTools.cpp          LineArcDrawing.cpp       Ruler.cpp                 WrappedCurves.cpp
        FastCurves.cpp         Loop.cpp                 SelectMode.cpp            wxImageLoader.cpp
        Geom.cpp               MagDragWindow.cpp        Shape.cpp
        glfont2.cpp            manager.cpp              ShapeData.cpp
   ) #Hole.cpp
set( HeeksCAD_LIBS
       -L../bin -ltinyxml -lheeksintf -lsketchsolve
   )
add_executable( heekscad ${heekscad_SRCS} ${platform_SRCS} ${heekscad_HDRS} )
target_link_libraries( heekscad
                       ${wxWidgets_LIBRARIES}  ${OpenCASCADE_LIBRARIES}
                       ${PYTHON_LIBRARIES} ${HeeksCAD_LIBS} ${OSX_LIBS} )
set_target_properties( heekscad PROPERTIES VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
install( TARGETS heekscad DESTINATION bin )

file( GLOB bitmaps "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/*.png"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/*.svg"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/*.glf"
    )
install( FILES ${bitmaps} DESTINATION share/heekscad/bitmaps )

file( GLOB icons "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*.png" )
install( FILES ${icons} DESTINATION share/heekscad/icons )

file( GLOB menu_icon "${CMAKE_CURRENT_SOURCE_DIR}/../icons/HeeksCAD.svg" )
install( FILES ${menu_icon} DESTINATION share/icons/hicolor/scalable/apps ) # freedesktop.org standard location

file( GLOB desktop "${CMAKE_CURRENT_SOURCE_DIR}/../HeeksCAD.desktop" )
install( FILES ${desktop} DESTINATION share/applications )
