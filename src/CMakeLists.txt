
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
find_package( wxWidgets REQUIRED COMPONENTS base core gl aui )
find_package( PythonLibs REQUIRED )
include(${wxWidgets_USE_FILE})

find_package( libarea REQUIRED )

include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${wxWidgets_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS} . ${OPENGL_INCLUDE_DIR}
    ${libarea_SRC_DIR}
    )
set ( EXECUTABLE_OUTPUT_PATH ../bin )

set( libarea_HDRS
    ${libarea_SRC_DIR}/Arc.h
    ${libarea_SRC_DIR}/Area.h
    ${libarea_SRC_DIR}/AreaOrderer.h
    ${libarea_SRC_DIR}/Box2D.h
    ${libarea_SRC_DIR}/Circle.h
    ${libarea_SRC_DIR}/Curve.h
    ${libarea_SRC_DIR}/Point.h
    ${libarea_SRC_DIR}/clipper.hpp
    ${libarea_SRC_DIR}/kurve/geometry.h
    )

set( heekscad_HDRS
    ../interface/Box.h
    ../interface/DoubleInput.h
    ../interface/HDialogs.h
    ../interface/HeeksCADInterface.h
    ../interface/HeeksColor.h
    ../interface/HeeksObj.h
    ../interface/HeeksObjDlg.h
    ../interface/IdNamedObj.h
    ../interface/IdNamedObjList.h
    ../interface/InputMode.h
    ../interface/LeftAndRight.h
    ../interface/MarkedObject.h
    ../interface/Material.h
    ../interface/NiceTextCtrl.h
    ../interface/ObjList.h
    ../interface/Observer.h
    ../interface/PictureFrame.h
    ../interface/Property.h
    ../interface/PropertyCheck.h
    ../interface/PropertyChoice.h
    ../interface/PropertyColor.h
    ../interface/PropertyDouble.h
    ../interface/PropertyInt.h
    ../interface/PropertyLength.h
    ../interface/PropertyList.h
    ../interface/PropertyString.h
    ../interface/PropertyVertex.h
    ../interface/Tool.h
    ../interface/ToolImage.h
    ../interface/ToolList.h
    ../interface/strconv.h
    ../tinyxml/tinystr.h
    ../tinyxml/tinyxml.h
    AboutBox.h
    AutoSave.h
    BezierCurve.h
    Cone.h
    ConversionTools.h
    CoordinateSystem.h
    CorrelationTool.h
    Cuboid.h
    Cylinder.h
    DigitizeMode.h
    DigitizedPoint.h
    DimensionDrawing.h
    Drawing.h
    Edge.h
    EndedObject.h
    Face.h
    FaceTools.h
    FastCurves.h
    GLList.h
    Geom.h
    GraphicsCanvas.h
    Grid.h
    Gripper.h
    GripperSelTransform.h
    Group.h
    HAngularDimension.h
    HArc.h
    HArea.h
    HCircle.h
    HDimension.h
    HDxf.h
    HEllipse.h
    HGear.h
    HILine.h
    HImage.h
    HLine.h
    HPoint.h
    HSpline.h
    HText.h
    HXml.h
    HeeksCAD.h
    HeeksConfig.h
    HeeksFrame.h
    HeeksPrintout.h
    History.h
    Index.h
    InputModeCanvas.h
    Intersector.h
    LineArcDrawing.h
    Loop.h
    MagDragWindow.h
    MarkedList.h
    ObjPropsCanvas.h
    OptionsCanvas.h
    OrientationModifier.h
    Plugins.h
    PointDrawing.h
    PointOrWindow.h
    PropertiesCanvas.h
    PropertyChange.h
    PropertyTrsf.h
    RegularShapesDrawing.h
    RemoveOrAddTool.h
    RuledSurface.h
    Ruler.h
    Sectioning.h
    SelectMode.h
    Shape.h
    ShapeData.h
    ShapeTools.h
    Sketch.h
    Solid.h
    SolidTools.h
    Sphere.h
    StlSolid.h
    StretchTool.h
    TransformTool.h
    TransformTools.h
    TreeCanvas.h
    Vertex.h
    ViewPanning.h
    ViewPoint.h
    ViewRotating.h
    ViewZooming.h
    Window.h
    Wire.h
    advprops.h
    dxf.h
    glfont2.h
    manager.h
    odcombo.h
    propdev.h
    propgrid.h
    stdafx.h
    svg.h
    wxImageLoader.h
    )

set( libarea_SRCS
    ${libarea_SRC_DIR}/Arc.cpp
    ${libarea_SRC_DIR}/Area.cpp
    ${libarea_SRC_DIR}/AreaClipper.cpp
    ${libarea_SRC_DIR}/AreaOrderer.cpp
    ${libarea_SRC_DIR}/AreaPocket.cpp
    ${libarea_SRC_DIR}/Circle.cpp
    ${libarea_SRC_DIR}/Curve.cpp
    ${libarea_SRC_DIR}/clipper.cpp
    ${libarea_SRC_DIR}/kurve/Construction.cpp
    ${libarea_SRC_DIR}/kurve/Finite.cpp
    ${libarea_SRC_DIR}/kurve/Matrix.cpp
    ${libarea_SRC_DIR}/kurve/kurve.cpp
    ${libarea_SRC_DIR}/kurve/offset.cpp
    )

set( heekscad_SRCS
    ../interface/DoubleInput.cpp
    ../interface/HDialogs.cpp
    ../interface/HeeksCADInterface.cpp
    ../interface/HeeksColor.cpp
    ../interface/HeeksObj.cpp
    ../interface/IdNamedObj.cpp
    ../interface/IdNamedObjList.cpp
    ../interface/LeftAndRight.cpp
    ../interface/MarkedObject.cpp
    ../interface/NiceTextCtrl.cpp
    ../interface/ObjList.cpp
    ../interface/PictureFrame.cpp
    ../interface/PropertyCheck.cpp
    ../interface/PropertyChoice.cpp
    ../interface/PropertyColor.cpp
    ../interface/PropertyDouble.cpp
    ../interface/PropertyInt.cpp
    ../interface/PropertyLength.cpp
    ../interface/PropertyList.cpp
    ../interface/PropertyString.cpp
    ../interface/PropertyVertex.cpp
    ../interface/ToolImage.cpp
    ../interface/ToolList.cpp
    ../interface/strconv.cpp
    ../tinyxml/tinystr.cpp
    ../tinyxml/tinyxml.cpp
    ../tinyxml/tinyxmlerror.cpp
    ../tinyxml/tinyxmlparser.cpp
    AboutBox.cpp
    AutoSave.cpp
    BezierCurve.cpp
    Cone.cpp
    ConversionTools.cpp
    CoordinateSystem.cpp
    CorrelationTool.cpp
    Cuboid.cpp
    CxfFont.cpp
    Cylinder.cpp
    DigitizeMode.cpp
    DigitizedPoint.cpp
    DimensionDrawing.cpp
    Drawing.cpp
    Edge.cpp
    EndedObject.cpp
    ExitMainLoop.cpp
    Face.cpp
    FaceTools.cpp
    FastCurves.cpp
    GLList.cpp
    Geom.cpp
    GraphicsCanvas.cpp
    Grid.cpp
    Gripper.cpp
    GripperSelTransform.cpp
    Group.cpp
    HAngularDimension.cpp
    HArc.cpp
    HArea.cpp
    HCircle.cpp
    HDimension.cpp
    HDxf.cpp
    HEllipse.cpp
    HGear.cpp
    HILine.cpp
    HImage.cpp
    HLine.cpp
    HPoint.cpp
    HSpline.cpp
    HText.cpp
    HXml.cpp
    HeeksCAD.cpp
    HeeksFrame.cpp
    HeeksPrintout.cpp
    History.cpp
    Input.cpp
    InputModeCanvas.cpp
    LineArcDrawing.cpp
    Loop.cpp
    MagDragWindow.cpp
    MarkedList.cpp
    ObjPropsCanvas.cpp
    OptionsCanvas.cpp
    OrientationModifier.cpp
    Plugins.cpp
    PointDrawing.cpp
    PointOrWindow.cpp
    PropertiesCanvas.cpp
    PropertyChange.cpp
    PropertyTrsf.cpp
    RS274X.cpp
    RegularShapesDrawing.cpp
    RemoveOrAddTool.cpp
    RuledSurface.cpp
    Ruler.cpp
    Sectioning.cpp
    SelectMode.cpp
    Shape.cpp
    ShapeData.cpp
    ShapeTools.cpp
    Sketch.cpp
    Solid.cpp
    SolidTools.cpp
    Sphere.cpp
    StlSolid.cpp
    StretchTool.cpp
    TransformTool.cpp
    TransformTools.cpp
    TreeCanvas.cpp
    Vertex.cpp
    ViewPanning.cpp
    ViewPoint.cpp
    ViewRotating.cpp
    ViewZooming.cpp
    Window.cpp
    Wire.cpp
    advprops.cpp
    dxf.cpp
    glfont2.cpp
    manager.cpp
    odcombo.cpp
    propgrid.cpp
    props.cpp
    stdafx.cpp
    svg.cpp
    wxImageLoader.cpp
    )

# remove lsketchsolve below
# -L../bin -ltinyxml -lheeksintf -lsketchsolve
set( HeeksCAD_LIBS
       -L../bin -ltinyxml -lheeksintf
    )
add_executable( heekscad ${heekscad_SRCS} ${libarea_SRCS} ${platform_SRCS} ${heekscad_HDRS} )
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

