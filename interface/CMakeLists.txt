project( heeksintf )
cmake_minimum_required( VERSION 2.6 )

add_definitions ( -Wall -DHEEKSCAD -DUNICODE -DTIXML_USE_STL )

find_package( OCE REQUIRED COMPONENTS TKG3d TKBRep )

# wxWidgets
find_package( wxWidgets REQUIRED COMPONENTS base core gl )
include(${wxWidgets_USE_FILE})

# python libs
find_package( PythonLibs REQUIRED )

include_directories ( ${CMAKE_SOURCE_DIR}
                      ${CMAKE_SOURCE_DIR}/src
                      ${PYTHON_INCLUDE_DIRS}
                      ${wxWidgets_INCLUDE_DIRS}
                      ${libarea_INCLUDE_DIRS}
                      ${OCE_INCLUDE_DIRS}
                    )
set ( LIBRARY_OUTPUT_PATH ../bin )

set( heeksintf_SRCS
    DoubleInput.cpp
    Geom.cpp
    HDialogs.cpp
    HeeksCADInterface.cpp
    HeeksColor.cpp
    HeeksObj.cpp
    HeeksObjDlg.cpp
    IdNamedObj.cpp
    IdNamedObjList.cpp
    LeftAndRight.cpp
    MarkedObject.cpp
    NiceTextCtrl.cpp
    ObjList.cpp
    PictureFrame.cpp
    PropertyCheck.cpp
    PropertyChoice.cpp
    PropertyColor.cpp
    PropertyDouble.cpp
    PropertyFile.cpp
    PropertyInt.cpp
    PropertyLength.cpp
    PropertyList.cpp
    PropertyString.cpp
    PropertyVertex.cpp
    ToolImage.cpp
    ToolList.cpp
    strconv.cpp
   )
   
set( heeksintf_HDRS
    Box.h
    DoubleInput.h
    Geom.h
    GripData.h
    GripperTypes.h
    HDialogs.h
    HeeksCADInterface.h
    HeeksColor.h
    HeeksObj.h
    HeeksObjDlg.h
    IdNamedObj.h
    IdNamedObjList.h
    InputMode.h
    LeftAndRight.h
    MarkedObject.h
    Material.h
    NiceTextCtrl.h
    NurbSurfaceParams.h
    ObjList.h
    Observer.h
    PictureFrame.h
    Plugin.h
    Property.h
    PropertyCheck.h
    PropertyChoice.h
    PropertyColor.h
    PropertyDouble.h
    PropertyFile.h
    PropertyInt.h
    PropertyLength.h
    PropertyList.h
    PropertyString.h
    PropertyVertex.h
    SketchOrder.h
    Tool.h
    ToolImage.h
    ToolList.h
    strconv.h
   )

if( APPLE )
add_library( heeksintf STATIC ${heeksintf_SRCS} )
else( APPLE )
add_library( heeksintf SHARED ${heeksintf_SRCS} )
endif( APPLE )

target_link_libraries( heeksintf ${PYTHON_LIBRARIES} ${wxWidgets_LIBRARIES} ${libarea_LIBRARIES} ${OCE_LIBRARIES} )

set_target_properties( heeksintf PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
install( TARGETS heeksintf DESTINATION ${LIB_INSTALL_DIR} )

file( GLOB sources ${heeksintf_SRCS} ${heeksintf_HDRS} )
install( FILES ${sources} DESTINATION "share/heekscad/interface" COMPONENT heeksintf )
