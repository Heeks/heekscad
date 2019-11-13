project( heekstinyxml )

cmake_minimum_required( VERSION 2.6 )

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

install(TARGETS heekstinyxml DESTINATION ${LIB_INSTALL_DIR} )

file( GLOB sources ${heekstinyxml_HDRS} )
# FIXME: to install in right directory (ie. include/heekstinyxml), sources must be updated to use "heekstinyxml/tinyxml.h"
install( FILES ${sources} DESTINATION "include/heekscad/tinyxml/" COMPONENT heekstinyxml )

if( WIN32 )
  install( FILES ${sources} DESTINATION "share/heekscad/tinyxml/" COMPONENT heekstinyxml )
else()
  # FIXME: a single step symlink installation isn't currently feasible
  # in Gentoo: using CMAKE_INSTALL_PREFIX in create_symlink will break
  # the sandbox, since it points to the real system. This explains why
  # the two steps below are necessary: 1. create symlink (the variable
  # CMAKE_CURRENT_BINARY_DIR points into the sandbox), 2. install link
  # into the system (the last row is automatically interpreted to work
  # into the sandbox; Gentoo will do a real install by its own will).
  install( CODE "execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink \
    \"${CMAKE_INSTALL_PREFIX}/include/heekscad/tinyxml\" \"${CMAKE_CURRENT_BINARY_DIR}/tinyxml\" )" COMPONENT heekstinyxml )
  install( FILES "${CMAKE_CURRENT_BINARY_DIR}/tinyxml" DESTINATION "share/heekscad/" COMPONENT heekstinyxml )
endif()
