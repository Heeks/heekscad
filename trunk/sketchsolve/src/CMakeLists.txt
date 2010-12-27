project( sketchsolve )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Debug )
add_definitions ( -Wall -DHEEKSCAD -DUNICODE -DTIXML_USE_STL )
#FIND_PACKAGE( wxWidgets REQUIRED COMPONENTS base core gl )
#INCLUDE(${wxWidgets_USE_FILE})
find_package( PythonLibs REQUIRED )
include_directories ( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${PYTHON_INCLUDE_DIRS} )
set ( LIBRARY_OUTPUT_PATH ../../bin )

set( sketchsolve_SRCS errorfuncs.cpp solve.cpp solveimpl.cpp ) #Derivatives.cpp SolverPointers.cpp
set( sketchsolve_HDRS timer.h solve.h )

add_library( sketchsolve SHARED ${sketchsolve_SRCS} ${sketchsolve_HDRS} )
target_link_libraries( sketchsolve ${PYTHON_LIBRARIES} )
set_target_properties( sketchsolve PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
install( TARGETS sketchsolve DESTINATION lib )
