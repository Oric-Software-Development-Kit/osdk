cmake_minimum_required(VERSION 3.15)
project(compression_test NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME HIPIC ADDRESS "$600" SOURCES main.c picture.s COMPILER_FLAGS -O3 PICTURE "../../data/picture.png" PICTURE_MODE PACKED PICTURE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/build/picture.s")
