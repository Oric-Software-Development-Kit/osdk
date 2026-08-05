cmake_minimum_required(VERSION 3.15)
project(hello_world_simple NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME HWSIMPLE ADDRESS "$800" SOURCES main.c COMPILER_FLAGS -O3)
