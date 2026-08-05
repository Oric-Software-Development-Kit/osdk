include_guard(GLOBAL)
set(OSDK_SAMPLE_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

# Set OSDK_ROOT explicitly, or export OSDK before configuring a sample.
if(NOT DEFINED OSDK_ROOT)
    if(DEFINED ENV{OSDK} AND NOT "$ENV{OSDK}" STREQUAL "")
        set(OSDK_ROOT "$ENV{OSDK}")
    else()
        message(FATAL_ERROR "Set OSDK_ROOT or the OSDK environment variable to an installed OSDK directory")
    endif()
endif()

file(TO_CMAKE_PATH "${OSDK_ROOT}" OSDK_ROOT)
find_program(OSDK_CPP NAMES clang cc gcc cpp REQUIRED)

function(osdk_add_sample)
    cmake_parse_arguments(ARG "" "NAME;ADDRESS;TAP_NAME;DISK_NAME;PICTURE;PICTURE_MODE;PICTURE_OUTPUT" "SOURCES;COMPILER_FLAGS;LINK_FLAGS;XA_FLAGS" ${ARGN})
    if(NOT ARG_NAME OR NOT ARG_SOURCES)
        message(FATAL_ERROR "osdk_add_sample requires NAME and SOURCES")
    endif()
    if(NOT ARG_ADDRESS)
        set(ARG_ADDRESS "$600")
    endif()
    if(NOT ARG_TAP_NAME)
        set(ARG_TAP_NAME "${ARG_NAME}")
    endif()

    set(_sources)
    set(_dependencies)
    foreach(_source IN LISTS ARG_SOURCES)
        set(_source_path "${CMAKE_CURRENT_SOURCE_DIR}/${_source}")
        list(APPEND _sources "${_source_path}")
        if(EXISTS "${_source_path}")
            list(APPEND _dependencies "${_source_path}")
        endif()
    endforeach()
    set(_tap "${CMAKE_CURRENT_BINARY_DIR}/build/${ARG_NAME}.tap")
    add_custom_command(
        OUTPUT "${_tap}"
        COMMAND "${CMAKE_COMMAND}"
            "-DOSDK_ROOT=${OSDK_ROOT}"
            "-DOSDK_CPP=${OSDK_CPP}"
            "-DSAMPLE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}/build"
            "-DNAME=${ARG_NAME}"
            "-DTAP_NAME=${ARG_TAP_NAME}"
            "-DADDRESS=${ARG_ADDRESS}"
            "-DSOURCES=${_sources}"
            "-DCOMPILER_FLAGS=${ARG_COMPILER_FLAGS}"
            "-DLINK_FLAGS=${ARG_LINK_FLAGS}"
            "-DXA_FLAGS=${ARG_XA_FLAGS}"
            "-DPICTURE=${ARG_PICTURE}"
            "-DPICTURE_MODE=${ARG_PICTURE_MODE}"
            "-DPICTURE_OUTPUT=${ARG_PICTURE_OUTPUT}"
            "-DDISK_NAME=${ARG_DISK_NAME}"
            -P "${OSDK_SAMPLE_MODULE_DIR}/OSDKSampleBuild.cmake"
        DEPENDS ${_dependencies} "${OSDK_SAMPLE_MODULE_DIR}/OSDKSampleBuild.cmake"
        COMMENT "Building ${ARG_NAME}.tap with OSDK"
        VERBATIM
    )
    add_custom_target(${ARG_NAME} ALL DEPENDS "${_tap}")
    if(ARG_DISK_NAME)
        add_custom_target(${ARG_NAME}_dsk ALL)
        add_dependencies(${ARG_NAME}_dsk ${ARG_NAME})
    endif()
endfunction()
