# CMakeLists.txt (Root of OSDK Project)
# =============================================================
# Cross-platform CMake configuration for OSDK.
# Supports Linux, macOS and Windows while automatically locating
# dependencies like FreeImage and Curses.
# =============================================================

cmake_minimum_required(VERSION 3.15)

project(OSDK LANGUAGES C CXX)

# =============================================================
# Installation Directory (OSDK Environment Variable Support)
# =============================================================

if(DEFINED ENV{OSDK} AND NOT "$ENV{OSDK}" STREQUAL "")
    set(CMAKE_INSTALL_PREFIX "$ENV{OSDK}" CACHE PATH "OSDK Installation Directory" FORCE)
elseif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/distributable/OSDK2.0/" CACHE PATH "OSDK Installation Directory" FORCE)
endif()

message(STATUS "OSDK Installation Prefix: ${CMAKE_INSTALL_PREFIX}")

# =============================================================
# Compiler Configuration
# =============================================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS __cdecl=)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall)
    add_compile_definitions(POSIX __cdecl=)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-implicit-function-declaration>)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
        add_compile_definitions(_DEBUG)
    else()
        add_compile_definitions(NDEBUG)
    endif()
endif()

# =============================================================
# Dependencies: Curses & FreeImage
# =============================================================

if(NOT WIN32)
    find_package(Curses REQUIRED)
endif()

find_package(FreeImage QUIET CONFIG)

if(TARGET FreeImage::FreeImage)
    message(STATUS "Found FreeImage via CMake package.")
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PC_FREEIMAGE QUIET FreeImage)
    endif()

    find_path(FREEIMAGE_INCLUDE_DIR
        NAMES
            FreeImage.h
        HINTS
            ${PC_FREEIMAGE_INCLUDEDIR}
            ${PC_FREEIMAGE_INCLUDE_DIRS}
            ${CMAKE_SOURCE_DIR}/shared_libraries/freeimage/v3.12.0
            ${CMAKE_SOURCE_DIR}/shared_libraries/freeimage/v3.12.0/Source
        PATH_SUFFIXES
            include
            Source
    )

    find_library(FREEIMAGE_LIBRARY
        NAMES
            FreeImage
            freeimage
        HINTS
            ${PC_FREEIMAGE_LIBDIR}
            ${PC_FREEIMAGE_LIBRARY_DIRS}
            ${CMAKE_SOURCE_DIR}/shared_libraries/freeimage/v3.12.0
            ${CMAKE_SOURCE_DIR}/shared_libraries/freeimage/v3.12.0/Dist
        PATH_SUFFIXES
            lib
            lib64
            Dist
    )

    include(FindPackageHandleStandardArgs)

    find_package_handle_standard_args(
        FreeImage
        REQUIRED_VARS
            FREEIMAGE_INCLUDE_DIR
            FREEIMAGE_LIBRARY
    )

    if(FreeImage_FOUND)
        add_library(FreeImage::FreeImage UNKNOWN IMPORTED)
        set_target_properties(FreeImage::FreeImage PROPERTIES
            IMPORTED_LOCATION "${FREEIMAGE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${FREEIMAGE_INCLUDE_DIR}"
        )
        message(STATUS "FreeImage found:")
        message(STATUS "  Include: ${FREEIMAGE_INCLUDE_DIR}")
        message(STATUS "  Library: ${FREEIMAGE_LIBRARY}")
    endif()
endif()

if(NOT TARGET FreeImage::FreeImage)
    message(FATAL_ERROR
        "FreeImage could not be located.\n"
        "Install the FreeImage development package or specify:\n"
        "  FREEIMAGE_INCLUDE_DIR\n"
        "  FREEIMAGE_LIBRARY"
    )
endif()

# =============================================================
# Subdirectories
# =============================================================

add_subdirectory(shared_libraries)
add_subdirectory(euphoric_tools)
add_subdirectory(osdk)

# Build the complete project, then install it.  The generated script resolves
# DESTINATION and OSDK when this target is invoked, so an existing build tree
# never falls back to its cached /tmp/osdk prefix when the environment changes.
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/BuildAndInstall.cmake.in"
    "${CMAKE_BINARY_DIR}/BuildAndInstall.cmake"
    @ONLY
)
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/BuildSamples.cmake.in"
    "${CMAKE_BINARY_DIR}/BuildSamples.cmake"
    @ONLY
)

# Invoke with: cmake --build <build-dir> --target build_and_install
add_custom_target(build_and_install
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/BuildAndInstall.cmake"
    COMMENT "Building all targets and installing OSDK"
    VERBATIM
)

# Build every CMake-based example from the toolchain just installed by the
# target above. Invoke with: cmake --build <build-dir> --target samples
add_custom_target(samples
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/BuildSamples.cmake"
    COMMENT "Configuring and building installed OSDK samples"
    VERBATIM
)
add_dependencies(samples build_and_install)
