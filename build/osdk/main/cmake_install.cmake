# Install script for directory: /Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/tmp/osdk")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/common/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/bas2tap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/bin2txt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/compiler/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/DskTool/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/filepack/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/FloppyBuilder/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/header/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/link65/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/macrosplitter/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/MemMap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/old2mfm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/opt65/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/pictconv/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/tap2cd/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/tap2dsk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/TapTool/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/xa/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/Ym2Mym/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE DIRECTORY FILES "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main/Osdk/_final_/documentation")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE DIRECTORY FILES "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main/Osdk/_final_/include")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE DIRECTORY FILES "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main/Osdk/_final_/lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE DIRECTORY FILES "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main/Osdk/_final_/macro")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE DIRECTORY FILES "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main/Osdk/_final_/Roms")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE DIRECTORY FILES "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main/Osdk/_final_/sample")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE DIRECTORY FILES "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main/Osdk/_final_/TMP")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}" TYPE FILE FILES "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/osdk/main/Osdk/_final_/read me.txt")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
          message(STATUS "Creating OSDK symlinks in ${CMAKE_INSTALL_PREFIX}...")
        if(UNIX)
            execute_process(COMMAND ln -sfn bin "${CMAKE_INSTALL_PREFIX}/Bin" ERROR_QUIET)
            execute_process(COMMAND ln -sfn MACROS.H "${CMAKE_INSTALL_PREFIX}/macro/macros.h" ERROR_QUIET)
            execute_process(COMMAND ln -sfn Roms "${CMAKE_INSTALL_PREFIX}/roms" ERROR_QUIET)
            execute_process(COMMAND ln -sfn TMP "${CMAKE_INSTALL_PREFIX}/tmp" ERROR_QUIET)
        else()
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink bin "${CMAKE_INSTALL_PREFIX}/Bin")
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink MACROS.H "${CMAKE_INSTALL_PREFIX}/macro/macros.h")
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink Roms "${CMAKE_INSTALL_PREFIX}/roms")
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink TMP "${CMAKE_INSTALL_PREFIX}/tmp")
        endif()

        foreach(B Bin2Tap DskTool FloppyBuilder TapTool Ym2Mym)
            string(TOLOWER "${B}" b)
            if(EXISTS "${CMAKE_INSTALL_PREFIX}/bin/${B}" AND NOT "${B}" STREQUAL "${b}")
                if(UNIX)
                    execute_process(COMMAND ln -sfn "${B}" "${CMAKE_INSTALL_PREFIX}/bin/${b}" ERROR_QUIET)
                else()
                    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${B}" "${CMAKE_INSTALL_PREFIX}/bin/${b}")
                endif()
            endif()
        endforeach()
    
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/j/Documents/Development/seclorum/retro/oric/osdk/osdk.seclorum/build/osdk/main/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
