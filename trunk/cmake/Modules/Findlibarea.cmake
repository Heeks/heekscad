# - Try to find the libarea sources
#
# Once done, this will define
#  libarea_FOUND - true if libarea has been found
#  libarea_SRC_DIR - the libarea src directory (*.h/*.cpp)
#
# Author: Romuald Conty <neomilium@gmail.com>
# Version: 20140512
#

IF(NOT libarea_FOUND)
  # Will try to find at standard locations
  FIND_PATH(libarea_SRC_DIR Area.cpp PATH_SUFFIXES libarea)

  IF( libarea_SRC_DIR STREQUAL libarea_SRC_DIR-NOTFOUND )
    # try to find at ./libarea/ location then ../libarea
    FIND_PATH( libarea_SRC_DIR Area.cpp PATHS "${CMAKE_SOURCE_DIR}/libarea" "${CMAKE_SOURCE_DIR}/../libarea" DOC "Path to libarea sources" )
    IF( libarea_SRC_DIR STREQUAL Area.cpp-NOTFOUND )
      MESSAGE( FATAL_ERROR "Cannot find libarea sources dir." )
    ENDIF( libarea_SRC_DIR STREQUAL Area.cpp-NOTFOUND )
  ENDIF(libarea_SRC_DIR STREQUAL libarea_SRC_DIR-NOTFOUND )

  set( libarea_FOUND TRUE )
  MESSAGE( STATUS "libarea_SRC_DIR:     " ${libarea_SRC_DIR} )
ENDIF(NOT libarea_FOUND)

