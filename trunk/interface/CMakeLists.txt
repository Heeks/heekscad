project( heeksintf )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Debug )
add_definitions ( -Wall -DHEEKSCAD -DUNICODE -DTIXML_USE_STL )
FIND_PACKAGE( wxWidgets REQUIRED COMPONENTS base core gl )
INCLUDE(${wxWidgets_USE_FILE})
find_package( PythonLibs REQUIRED )
include_directories ( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                      ${PYTHON_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS}
                      ../src
                    )
set ( LIBRARY_OUTPUT_PATH ../bin )

set( heeksintf_SRCS
        DoubleInput.cpp        NiceTextCtrl.cpp    PropertyDouble.cpp  strconv.cpp
        HDialogs.cpp           ObjectCanvas.cpp    PropertyFile.cpp    ToolImage.cpp
        HeeksCADInterface.cpp  ObjList.cpp         PropertyInt.cpp     ToolList.cpp
        HeeksColor.cpp         PictureFrame.cpp    PropertyLength.cpp  TransientObject.cpp
        HeeksObj.cpp           PropertyCheck.cpp   PropertyList.cpp
        LeftAndRight.cpp       PropertyChoice.cpp  PropertyString.cpp
        MarkedObject.cpp       PropertyColor.cpp   PropertyVertex.cpp
   )
   
set( heeksintf_HDRS
        Box.h                InputMode.h          Observer.h        Property.h        Tool.h
        DoubleInput.h        LeftAndRight.h       PictureFrame.h    PropertyInt.h     ToolImage.h
        GripData.h           MarkedObject.h       Plugin.h          PropertyLength.h  ToolList.h
        GripperTypes.h       Material.h           PropertyCheck.h   PropertyList.h    TransientObject.h
        HDialogs.h           NiceTextCtrl.h       PropertyChoice.h  PropertyString.h
        HeeksCADInterface.h  NurbSurfaceParams.h  PropertyColor.h   PropertyVertex.h
        HeeksColor.h         ObjectCanvas.h       PropertyDouble.h  SketchOrder.h
        HeeksObj.h           ObjList.h            PropertyFile.h    strconv.h
   )

add_library( heeksintf SHARED ${heeksintf_SRCS} ${heeksintf_HDRS} )
target_link_libraries( heeksintf ${PYTHON_LIBRARIES} ${wxWidgets_LIBRARIES} )
set_target_properties( heeksintf PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
install( TARGETS heeksintf DESTINATION lib )
