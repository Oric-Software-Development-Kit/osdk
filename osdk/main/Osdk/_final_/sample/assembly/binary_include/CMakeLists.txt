cmake_minimum_required(VERSION 3.15)
project(binary_include NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME BININCL ADDRESS "$600" SOURCES main.s PICTURE "../../data/picture.png")
