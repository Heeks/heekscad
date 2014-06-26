project( heekscad )
cmake_minimum_required( VERSION 2.6 )

add_definitions ( -Wall -DUSE_GENERIC_TREECTRL -DHEEKSCAD -DUNICODE -DTIXML_USE_STL )

#find OCE or OpenCASCADE
set( CASCADE_LIBS TKBRep TKIGES TKSTEP TKXSBase TKBool TKernel TKFillet TKG2d TKG3d TKGeomAlgo TKGeomBase TKMath TKOffset TKPrim TKTopAlgo TKV3d TKMesh TKBO )
set( CASCADE_NOTUSED_LIBS TKPShape TKShHealing TKLCAF TKFeat TKXCAF TKHLR TKPLCAF TKSTL TKSTEP209 TKAdvTools TKCAF TKV2d TKVRML TKXCAFSchema TKXDESTEP TKSTEPAttr FWOSPlugin TKSTEPBase TKService TKShapeSchema TKCDF )
#inherits variables from parent dir - don't need to 'find_package ( OCE )' again
if( ${OCE_FOUND} )
  set( OpenCASCADE_LIBRARIES ${CASCADE_LIBS} )
  set( OpenCASCADE_INCLUDE_DIR ${OCE_INCLUDE_DIRS} )
else() #look for OpenCASCADE
  find_package ( OpenCASCADE REQUIRED COMPONENTS ${CASCADE_LIBS} )
endif()

find_package( OpenGL REQUIRED )
find_package( wxWidgets REQUIRED COMPONENTS base core gl aui )
find_package( PythonLibs REQUIRED )

include(${wxWidgets_USE_FILE})

include_directories ( SYSTEM
    ${OpenCASCADE_INCLUDE_DIR}
    )

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${PYTHON_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${wxWidgets_INCLUDE_DIRS}
    ${libarea_INCLUDE_DIRS}
    )

set ( EXECUTABLE_OUTPUT_PATH ../bin )

set( heekscad_HDRS
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

set( heekscad_SRCS
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
# -L../bin -ltinyxml -lheeksintf
set( HeeksCAD_LIBS
       -L../bin -lheekstinyxml -lheeksintf
    )
add_executable( heekscad ${heekscad_SRCS} ${platform_SRCS} )
target_link_libraries( heekscad
                       ${wxWidgets_LIBRARIES} ${OpenCASCADE_LIBRARIES}
                       ${OPENGL_LIBRARIES} ${PYTHON_LIBRARIES} ${OSX_LIBS}
                       ${HeeksCAD_LIBS} ${libarea_LIBRARIES} )
message(STATUS "wxWidgets_LIBRARIES: ${wxWidgets_LIBRARIES}")
message(STATUS "wxWidgets_ROOT_DIR: ${wxWidgets_ROOT_DIR}")
message(STATUS "wxWidgets_INCLUDE_DIRS: ${wxWidgets_INCLUDE_DIRS}")

set_target_properties( heekscad PROPERTIES VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
install( TARGETS heekscad DESTINATION bin )

# Bitmaps, fonts, etc.
foreach( bitmap_relpath "" "angle" "cuboid" )
  file( GLOB bitmaps_${bitmap_relpath} "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/${bitmap_relpath}/*.png"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/${bitmap_relpath}/*.svg"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/${bitmap_relpath}/*.glf"
    )
  install( FILES ${bitmaps_${bitmap_relpath}} DESTINATION share/heekscad/bitmaps/${bitmap_relpath} )
endforeach()

file( GLOB icons "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*.png" )
install( FILES ${icons} DESTINATION share/heekscad/icons )

file( GLOB menu_icon "${CMAKE_CURRENT_SOURCE_DIR}/../icons/HeeksCAD.svg" )
install( FILES ${menu_icon} DESTINATION share/icons/hicolor/scalable/apps ) # freedesktop.org standard location

file( GLOB desktop "${CMAKE_CURRENT_SOURCE_DIR}/../HeeksCAD.desktop" )
install( FILES ${desktop} DESTINATION share/applications )

