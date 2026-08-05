cmake_minimum_required(VERSION 3.15)
project(hello_world_advanced NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME HWADVANCED ADDRESS "$800" SOURCES main.c print.s)
