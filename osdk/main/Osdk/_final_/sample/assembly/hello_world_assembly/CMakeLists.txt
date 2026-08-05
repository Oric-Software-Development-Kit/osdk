cmake_minimum_required(VERSION 3.15)
project(hello_world_assembly NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME HWASM ADDRESS "$800" SOURCES main.s LINK_FLAGS -B)
