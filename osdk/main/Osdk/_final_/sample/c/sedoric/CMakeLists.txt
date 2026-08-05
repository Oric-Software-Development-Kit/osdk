cmake_minimum_required(VERSION 3.15)
project(sedoric NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME SEDDEMO TAP_NAME SEDDEMO ADDRESS "$600" SOURCES main.c DISK_NAME seddemo PICTURE "../../data/picture.png" PICTURE_MODE 1 PICTURE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/build/picture.tap")
