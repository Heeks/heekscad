project( heekscad-translations )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Debug )

#foreach ( _lang de fr it sk )
#  add_custom_command ( OUTPUT

# Adapted from http://techbase.kde.org/Development/Tutorials/Localization/i18n_Build_Systems#Compiling_and_installing_message_catalogs

FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

IF(NOT GETTEXT_MSGFMT_EXECUTABLE)
        MESSAGE(
"------
                 NOTE: msgfmt not found. Translations will *not* be installed
------")
ELSE(NOT GETTEXT_MSGFMT_EXECUTABLE)

        SET(appname HeeksCAD)

        ADD_CUSTOM_TARGET(translations ALL)

        FILE(GLOB PO_FILES */${appname}.po)

        FOREACH(_poFile ${PO_FILES})
                GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
                GET_FILENAME_COMPONENT(_poFilePath ${_poFile} PATH)
                #in the kde version of this file, the language code was part of the filename
                #for HeeksCAD, it's the subdir containing the .po file
                STRING(REGEX REPLACE "^.*\\/" "" _langCode ${_poFilePath} )

                IF( _langCode )
                        SET(_moFile ${CMAKE_CURRENT_BINARY_DIR}/${_langCode}/${appname}.mo)

                        ADD_CUSTOM_COMMAND(TARGET translations
                                COMMAND cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${_langCode}
                                COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_moFile} ${_poFile}
                                DEPENDS ${_poFile})
                        INSTALL(FILES ${_moFile} DESTINATION share/locale/${_langCode}/LC_MESSAGES/ )
                ENDIF( _langCode )

        ENDFOREACH(_poFile ${PO_FILES})

ENDIF(NOT GETTEXT_MSGFMT_EXECUTABLE)
