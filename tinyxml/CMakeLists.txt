project( heekstinyxml )

cmake_minimum_required( VERSION 2.6 )

set( CMAKE_BUILD_TYPE Debug )

add_definitions( -Wall -DHEEKSCAD -DUNICODE -DTIXML_USE_STL )

include_directories (
    ${CMAKE_SOURCE_DIR}
    ${PYTHON_INCLUDE_DIRS}
)

set( LIBRARY_OUTPUT_PATH ../bin )

set( heekstinyxml_SRCS
  tinystr.cpp
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
)

set( heekstinyxml_HDRS
  tinystr.h
  tinyxml.h
)

add_library( heekstinyxml SHARED ${heekstinyxml_SRCS} ${heekstinyxml_HDRS} )

target_link_libraries( heekstinyxml ${PYTHON_LIBRARIES} )

set_target_properties( heekstinyxml PROPERTIES SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )

install( TARGETS heekstinyxml DESTINATION lib )

file( GLOB sources ${heekstinyxml_HDRS} )
# FIXME: to install in right directory (ie. include/heekstinyxml), sources must be updated to use "heekstinyxml/tinyxml.h"
install( FILES ${sources} DESTINATION "include/heekscad/tinyxml/" COMPONENT heekstinyxml )
