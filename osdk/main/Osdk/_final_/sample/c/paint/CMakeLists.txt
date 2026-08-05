cmake_minimum_required(VERSION 3.15)
project(c_paint NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME PAINT ADDRESS "$800" SOURCES main.c picture.s COMPILER_FLAGS -O1 -g1 PICTURE "../../data/flood_fill_test.png" PICTURE_MODE 4 PICTURE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/build/picture.s")
