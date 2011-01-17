cmake_minimum_required( VERSION 2.6 )
if( ${CMAKE_VERSION} STREQUAL "2.8.2" )
  # bug http://vtk.org/Bug/view.php?id=11020
  message( WARNING "CMake/CPack version 2.8.2 will not create working .deb packages!")
endif( ${CMAKE_VERSION} STREQUAL "2.8.2" )

set( CMAKE_BUILD_TYPE Debug )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set( BUILD_STARTED_IN_RIGHT_DIR "true" )  #used in src/CMakeLists.txt to print a message

add_definitions ( -Wall )
FIND_PACKAGE( wxWidgets REQUIRED COMPONENTS base core gl )
find_package ( OpenCASCADE REQUIRED COMPONENTS TKernel )

if( APPLE )
  # Apple includes two different openGL implementations, for Carbon and X11.  We want X11
  set( OSX_INCLUDE_DIRS /usr/X11R6/include )
  set( OSX_LIBS -L/usr/X11R6/lib -lGL -lX11 -lGLU -lglut )
endif( APPLE )

INCLUDE(${wxWidgets_USE_FILE})
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${wxWidgets_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR}
	${OSX_INCLUDE_DIRS}
	)


IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
  set(PKG_ARCH i386)
ELSE( CMAKE_SIZEOF_VOID_P EQUAL 4 )
  set(PKG_ARCH amd64)
ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 4 )

#-------------------- package generation -------------------------
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "HeeksCAD Developers <heekscad-devel@googlegroups.com>")
set(CPACK_PACKAGE_NAME "heekscad")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 15)
set(CPACK_PACKAGE_VERSION_PATCH 1)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_beta-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}_${PKG_ARCH}")

#newlines in the summary MUST be followed by a space, or dpkg will refuse to install!
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HeeksCAD, Free CAD based on Open CASCADE
 HeeksCAD is a free 3D CAD program based on Open CASCADE.
 Available plugins include HeeksCNC (for g-code generation)
 and HeeksPython (for python scripting)")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_VERSION "")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libwxbase2.8-0 (>= 2.8.5), libwxgtk2.8-0 (>= 2.8.5), libgtkglext1 (>= 1.2), python (>= 2.5), libboost-python1.40.0 | libboost-python1.42.0
Recommends: libopencascade-ocaf-6.3.0")
# ^^^^^^ A newline is REQUIRED immediately before the keyword Recommends. This takes advantage of a loophole in CPack - it doesn't have a way to set recommended packages. I set OCC as a recommendation because some of us devs (me, anyway) built from source instead of installing packages

include(CPack)

#---------- put subdirs down here so that the package version vars above are visible to them -----------
add_subdirectory( interface )
add_subdirectory( tinyxml )
add_subdirectory( sketchsolve/src )
add_subdirectory( translations )
add_subdirectory( src )  #needs libraries from other subdirs, so it goes last

