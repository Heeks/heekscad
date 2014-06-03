# - Try to find the libarea sources
#
# Once done, this will define
#  libarea_FOUND - true if libarea has been found
#  libarea_INCLUDE_DIRS - the libarea include directories
#  libarea_LIBRARIES - the libarea libs
#
# Author: Romuald Conty <neomilium@gmail.com>
# Version: 20140515
#

IF(NOT libarea_FOUND)
  FIND_PACKAGE (PkgConfig)
    IF(PKG_CONFIG_FOUND)
    # Will find Area library on Linux/BSDs using PkgConfig
    PKG_CHECK_MODULES(libarea libarea)
  ENDIF(PKG_CONFIG_FOUND)
  
  IF(NOT libarea_FOUND)
    # Will find libarea headers
    FIND_PATH(libarea_INCLUDE_DIRS Area.h PATH_SUFFIXES area)
    FIND_LIBRARY(libarea_LIBRARIES NAMES area)

    IF(libarea_INCLUDE_DIRS AND libarea_LIBRARIES)
      SET(libarea_FOUND true)
    ENDIF(libarea_INCLUDE_DIRS AND libarea_LIBRARIES)
  ENDIF(NOT libarea_FOUND)
  
  MESSAGE( STATUS "libarea_INCLUDE_DIRS:     " ${libarea_INCLUDE_DIRS} )
  MESSAGE( STATUS "libarea_LIBRARIES:     " ${libarea_LIBRARIES} )
ENDIF(NOT libarea_FOUND)
