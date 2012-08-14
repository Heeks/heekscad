
project( heekscad )
cmake_minimum_required( VERSION 2.6 )

if( NOT DEFINED BUILD_STARTED_IN_RIGHT_DIR )
  message( FATAL_ERROR "Don't start a build in the src directory. Do a 'cd ..', then run cmake ." )
endif( NOT DEFINED BUILD_STARTED_IN_RIGHT_DIR )

set ( CMAKE_BUILD_TYPE Debug )
add_definitions ( -Wall -DUSE_GENERIC_TREECTRL -DHEEKSCAD -DUNICODE -DTIXML_USE_STL )

#find OCE or OpenCASCADE
set( CASCADE_LIBS TKVRML TKSTL TKBRep TKIGES TKShHealing TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKXSBase TKShapeSchema FWOSPlugin TKBool TKCAF TKCDF TKernel TKFeat TKFillet TKG2d TKG3d TKGeomAlgo TKGeomBase TKHLR TKMath TKOffset TKPrim TKPShape TKService TKTopAlgo TKV2d TKV3d TKMesh TKAdvTools TKBO TKXDESTEP TKXCAF TKXCAFSchema TKLCAF TKPLCAF )
#inherits variables from parent dir - don't need to 'find_package ( OCE )' again
if( ${OCE_FOUND} )
  set( OpenCASCADE_LIBRARIES ${CASCADE_LIBS} )
  set( OpenCASCADE_INCLUDE_DIR ${OCE_INCLUDE_DIRS} )
else() #look for OpenCASCADE
  set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
  find_package ( OpenCASCADE REQUIRED COMPONENTS ${CASCADE_LIBS} )
endif()

find_package( OpenGL REQUIRED )
FIND_PACKAGE( wxWidgets REQUIRED COMPONENTS base core gl aui )
find_package( PythonLibs REQUIRED )
INCLUDE(${wxWidgets_USE_FILE})

include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${wxWidgets_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS} . ${OPENGL_INCLUDE_DIR}
    )
set ( EXECUTABLE_OUTPUT_PATH ../bin )

set( LIBAREA_PATH ../libarea/ )

set( heekscad_HDRS
    AboutBox.h
    advprops.h
    ${LIBAREA_PATH}/Arc.h
    ${LIBAREA_PATH}/Area.h
    ${LIBAREA_PATH}/AreaOrderer.h
    AutoSave.h
    BentleyOttmann.h
    BezierCurve.h
    ../interface/Box.h
    ${LIBAREA_PATH}/Box2D.h
    ${LIBAREA_PATH}/Circle.h
    ${LIBAREA_PATH}/clipper.hpp
    Cone.h
    ConversionTools.h
    CoordinateSystem.h
    CorrelationTool.h
    Cuboid.h
    ${LIBAREA_PATH}/Curve.h
    CxfFont.h
    Cylinder.h
    DigitizedPoint.h
    DigitizeMode.h
    DimensionDrawing.h
    ../interface/DoubleInput.h
    Drawing.h
    dxf.h
    DynamicSolid.h
    Edge.h
    EndedObject.h
    Face.h
    FaceTools.h
    FastCurves.h
    Geom.h
    ${LIBAREA_PATH}/kurve/geometry.h
    glfont2.h
    GLList.h
    GraphicsCanvas.h
    Grid.h
    Gripper.h
    GripperSelTransform.h
    Group.h
    HAngularDimension.h
    HArc.h
    HArea.h
    HCircle.h
    ../interface/HDialogs.h
    HDimension.h
    HDxf.h
    HeeksCAD.h
    ../interface/HeeksCADInterface.h
    ../interface/HeeksColor.h
    HeeksConfig.h
    HeeksFrame.h
    ../interface/HeeksObj.h
    HeeksPrintout.h
    HEllipse.h
    HGear.h
    HILine.h
    HImage.h
    HLine.h
    HPoint.h
    HSpline.h
    HText.h
    HXml.h
    Index.h
    ../interface/InputMode.h
    InputModeCanvas.h
    Intersector.h
    ../interface/LeftAndRight.h
    LineArcDrawing.h
    Loop.h
    MagDragWindow.h
    manager.h
    MarkedList.h
    ../interface/MarkedObject.h
    ../interface/Material.h
    MultiPoly.h
    NearMap.h
    ../interface/NiceTextCtrl.h
    ../interface/ObjectCanvas.h
    ../interface/ObjList.h
    ObjPropsCanvas.h
    ../interface/Observer.h
    odcombo.h
    OptionsCanvas.h
    OrientationModifier.h
    Pad.h
    Part.h
    ../interface/PictureFrame.h
    Plugins.h
    Pocket.h
    ${LIBAREA_PATH}/Point.h
    PointDrawing.h
    PointOrWindow.h
    propdev.h
    PropertiesCanvas.h
    ../interface/Property.h
    ../interface/PropertyCheck.h
    ../interface/PropertyChoice.h
    ../interface/PropertyColor.h
    ../interface/PropertyDouble.h
    ../interface/PropertyInt.h
    ../interface/PropertyLength.h
    ../interface/PropertyList.h
    ../interface/PropertyString.h
    PropertyTrsf.h
    ../interface/PropertyVertex.h
    propgrid.h
    RegularShapesDrawing.h
    RuledSurface.h
    Ruler.h
    Sectioning.h
    SelectMode.h
    Shape.h
    ShapeData.h
    ShapeTools.h
    SimpleIntersector.h
    Sketch.h
    SketchTools.h
    Solid.h
    SolidTools.h
    Sphere.h
    stdafx.h
    StlSolid.h
    ../interface/strconv.h
    StretchTool.h
    svg.h
    ../tinyxml/tinystr.h
    ../tinyxml/tinyxml.h
    ../interface/Tool.h
    ../interface/ToolImage.h
    ../interface/ToolList.h
    TransformTool.h
    TransformTools.h
    ../interface/TransientObject.h
    TreeCanvas.h
    UndoEngine.h
    Vertex.h
    ViewPanning.h
    ViewPoint.h
    ViewRotating.h
    ViewZooming.h
    Window.h
    Wire.h
    WrappedCurves.h
    wxImageLoader.h
   )

set( heekscad_SRCS
    AboutBox.cpp
    advprops.cpp
    ${LIBAREA_PATH}/Arc.cpp
    ${LIBAREA_PATH}/Area.cpp
    ${LIBAREA_PATH}/AreaClipper.cpp
    ${LIBAREA_PATH}/AreaOrderer.cpp
    ${LIBAREA_PATH}/AreaPocket.cpp
    AutoSave.cpp
    BentleyOttmann.cpp
    BezierCurve.cpp
    ${LIBAREA_PATH}/Circle.cpp
    ${LIBAREA_PATH}/clipper.cpp
    Cone.cpp
    ${LIBAREA_PATH}/kurve/Construction.cpp
    ConversionTools.cpp
    CoordinateSystem.cpp
    CorrelationTool.cpp
    Cuboid.cpp
    ${LIBAREA_PATH}/Curve.cpp
    CxfFont.cpp
    Cylinder.cpp
    DigitizedPoint.cpp
    DigitizeMode.cpp
    DimensionDrawing.cpp
    ../interface/DoubleInput.cpp
    Drawing.cpp
    dxf.cpp
    DynamicSolid.cpp
    Edge.cpp
    EndedObject.cpp
    ExitMainLoop.cpp
    Face.cpp
    FaceTools.cpp
    FastCurves.cpp
    ${LIBAREA_PATH}/kurve/Finite.cpp
    Geom.cpp
    glfont2.cpp
    GLList.cpp
    GraphicsCanvas.cpp
    Grid.cpp
    Gripper.cpp
    GripperSelTransform.cpp
    Group.cpp
    HAngularDimension.cpp
    HArc.cpp
    HArea.cpp
    HCircle.cpp
    ../interface/HDialogs.cpp
    HDimension.cpp
    HDxf.cpp
    HeeksCAD.cpp
    ../interface/HeeksCADInterface.cpp
    ../interface/HeeksColor.cpp
    HeeksFrame.cpp
    ../interface/HeeksObj.cpp
    HeeksPrintout.cpp
    HEllipse.cpp
    HGear.cpp
    HILine.cpp
    HImage.cpp
    HLine.cpp
    HPoint.cpp
    HSpline.cpp
    HText.cpp
    HXml.cpp
    Input.cpp
    InputModeCanvas.cpp
    ${LIBAREA_PATH}/kurve/kurve.cpp
    ../interface/LeftAndRight.cpp
    LineArcDrawing.cpp
    Loop.cpp
    MagDragWindow.cpp
    manager.cpp
    MarkedList.cpp
    ../interface/MarkedObject.cpp
    ${LIBAREA_PATH}/kurve/Matrix.cpp
    MultiPoly.cpp
    NearMap.cpp
    ../interface/NiceTextCtrl.cpp
    ../interface/ObjList.cpp
    ObjPropsCanvas.cpp
    odcombo.cpp
    ${LIBAREA_PATH}/kurve/offset.cpp
    OptionsCanvas.cpp
    OrientationModifier.cpp
    Pad.cpp
    Part.cpp
    ../interface/PictureFrame.cpp
    Plugins.cpp
    Pocket.cpp
    PointDrawing.cpp
    PointOrWindow.cpp
    Polygon.cpp
    PropertiesCanvas.cpp
    ../interface/PropertyCheck.cpp
    ../interface/PropertyChoice.cpp
    ../interface/PropertyColor.cpp
    ../interface/PropertyDouble.cpp
    ../interface/PropertyInt.cpp
    ../interface/PropertyLength.cpp
    ../interface/PropertyList.cpp
    ../interface/PropertyString.cpp
    PropertyTrsf.cpp
    ../interface/PropertyVertex.cpp
    propgrid.cpp
    props.cpp
    RegularShapesDrawing.cpp
    RS274X.cpp
    RuledSurface.cpp
    Ruler.cpp
    Sectioning.cpp
    SelectMode.cpp
    Shape.cpp
    ShapeData.cpp
    ShapeTools.cpp
    SimpleIntersector.cpp
    Sketch.cpp
    SketchTools.cpp
    Solid.cpp
    SolidTools.cpp
    Sphere.cpp
    stdafx.cpp
    StlSolid.cpp
    ../interface/strconv.cpp
    StretchTool.cpp
    svg.cpp
    ../tinyxml/tinystr.cpp
    ../tinyxml/tinyxml.cpp
    ../tinyxml/tinyxmlerror.cpp
    ../tinyxml/tinyxmlparser.cpp
    ../interface/ToolImage.cpp
    ../interface/ToolList.cpp
    TransformTool.cpp
    TransformTools.cpp
    ../interface/TransientObject.cpp
    TreeCanvas.cpp
    UndoEngine.cpp
    Vertex.cpp
    ViewPanning.cpp
    ViewPoint.cpp
    ViewRotating.cpp
    ViewZooming.cpp
    Window.cpp
    Wire.cpp
    WrappedCurves.cpp
    wxImageLoader.cpp
   )

set( HeeksCAD_LIBS
       -L../bin -ltinyxml -lheeksintf -lsketchsolve
   )
add_executable( heekscad ${heekscad_SRCS} ${platform_SRCS} ${heekscad_HDRS} )
target_link_libraries( heekscad
                       ${wxWidgets_LIBRARIES}  ${OpenCASCADE_LIBRARIES}
                       ${PYTHON_LIBRARIES} ${HeeksCAD_LIBS} ${OSX_LIBS}
                       ${OPENGL_LIBRARIES} )
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

