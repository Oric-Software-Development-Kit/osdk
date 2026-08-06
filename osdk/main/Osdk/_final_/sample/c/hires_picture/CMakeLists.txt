cmake_minimum_required(VERSION 3.15)
project(hires_picture NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME HIPIC ADDRESS "$600" SOURCES main.c picture.s PICTURE "../../data/picture.png" PICTURE_MODE 4 PICTURE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/build/picture.s")
