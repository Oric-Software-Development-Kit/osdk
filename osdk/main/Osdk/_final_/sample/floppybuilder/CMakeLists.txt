cmake_minimum_required(VERSION 3.15)
project(floppybuilder_sample NONE)

# This project needs the generated loader layout, so it remains a dedicated
# target rather than using the single-program helper used by the other samples.
if(NOT DEFINED OSDK_ROOT)
    if(DEFINED ENV{OSDK} AND NOT "$ENV{OSDK}" STREQUAL "")
        set(OSDK_ROOT "$ENV{OSDK}")
    else()
        message(FATAL_ERROR "Set OSDK_ROOT or the OSDK environment variable to an installed OSDK directory")
    endif()
endif()

add_custom_target(FloppyBuilderSample
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/build/files"
    COMMAND "${OSDK_ROOT}/bin/pictconv" -f0 -d0 -o2 "${CMAKE_CURRENT_SOURCE_DIR}/data/Font6x8.png" "${CMAKE_CURRENT_BINARY_DIR}/build/files/Font6x8.hir"
    COMMAND "${OSDK_ROOT}/bin/pictconv" -f1 -d0 -o2 "${CMAKE_CURRENT_SOURCE_DIR}/data/FirstProgram.png" "${CMAKE_CURRENT_BINARY_DIR}/build/files/FirstProgram.hir"
    COMMAND "${OSDK_ROOT}/bin/pictconv" -f1 -d0 -o2 "${CMAKE_CURRENT_SOURCE_DIR}/data/SecondProgram.png" "${CMAKE_CURRENT_BINARY_DIR}/build/files/SecondProgram.hir"
    COMMAND "${OSDK_ROOT}/bin/FloppyBuilder" init "${CMAKE_CURRENT_SOURCE_DIR}/code/floppybuilderscript.txt"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Preparing FloppyBuilder sample assets with OSDK"
    VERBATIM
)
