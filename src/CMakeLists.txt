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


include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${wxWidgets_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS} .
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
        DynamicSolid.h       PropertiesCanvas.h		PropertyTrsf.h          ViewPoint.h
        EdgeBlend.h          HPoint.h               propgrid.h              ViewRotating.h
        Edge.h               HSpline.h              RegularShapesDrawing.h  ViewZooming.h
        EndedObject.h        HText.h                RS274X.h                Window.h
        Face.h               HXml.h                 RuledSurface.h          Wire.h
        FaceTools.h          InputModeCanvas.h      Ruler.h                 WrappedCurves.h
        FastCurves.h         Intersector.h          Sectioning.h            wxImageLoader.h
        Geom.h               LineArcDrawing.h       SelectMode.h            
   )#Hole.h

set( heekscad_SRCS
        AboutBox.cpp           GLList.cpp               manager.cpp               ShapeTools.cpp
        advprops.cpp           GraphicsCanvas.cpp       MarkedList.cpp            SimpleIntersector.cpp
        AutoSave.cpp           Grid.cpp                 matrix.cpp                ShapeTools.cpp     
        BentleyOttmann.cpp     Gripper.cpp              MultiPoly.cpp             SimpleIntersector.cpp
        BezierCurve.cpp        GripperSelTransform.cpp  NearMap.cpp               Sketch.cpp   
        Cone.cpp               Group.cpp                ObjPropsCanvas.cpp        sketchinterface.cpp
        ConstrainedObject.cpp  HAngularDimension.cpp    OCCSolver.cpp             SketchTools.cpp
        Constraint.cpp         HArc.cpp                 odcombo.cpp               Solid.cpp
        ConstraintTools.cpp    HCircle.cpp              OptionsCanvas.cpp         SolidTools.cpp
        ConversionTools.cpp    HDimension.cpp           OrientationModifier.cpp   SolveSketch.cpp
        CoordinateSystem.cpp   HDxf.cpp                 Pad.cpp                   Sphere.cpp
        CorrelationTool.cpp    HeeksCAD.cpp             Part.cpp                  stdafx.cpp
        Cuboid.cpp             HeeksFrame.cpp           Plugins.cpp               StlSolid.cpp
        CxfFont.cpp            HeeksPrintout.cpp        Pocket.cpp                StretchTool.cpp
        Cylinder.cpp           HEllipse.cpp             PointDrawing.cpp          svg.cpp
        DigitizedPoint.cpp     HILine.cpp               PointOrWindow.cpp         TransformTool.cpp
        DigitizeMode.cpp       HImage.cpp               Polygon.cpp               TransformTools.cpp
        DimensionDrawing.cpp   HLine.cpp                PropertiesCanvas.cpp      TreeCanvas.cpp
        Drawing.cpp            PropertyTrsf.cpp         propgrid.cpp              UndoEngine.cpp
        dxf.cpp                HPoint.cpp               props.cpp                 Vertex.cpp
        DynamicSolid.cpp       HSpline.cpp              RegularShapesDrawing.cpp  ViewPanning.cpp
        Edge.cpp               HText.cpp                RS274X.cpp                ViewPoint.cpp
        EndedObject.cpp        HXml.cpp                 RuledSurface.cpp          ViewRotating.cpp
        Face.cpp               Input.cpp                Ruler.cpp                 ViewZooming.cpp
        FaceTools.cpp          InputModeCanvas.cpp      SelectMode.cpp            Window.cpp
        FastCurves.cpp         LineArcDrawing.cpp       Shape.cpp                 Wire.cpp
        Geom.cpp               Loop.cpp                 ShapeData.cpp             WrappedCurves.cpp           
        glfont2.cpp            MagDragWindow.cpp                                  wxImageLoader.cpp      
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
