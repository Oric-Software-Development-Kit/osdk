cmake_minimum_required(VERSION 3.15)
project(debug_type_zoo NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME DBGZOO ADDRESS "$800" SOURCES main.c world.c asm_data.s asm_helpers.s COMPILER_FLAGS -O2 -g1)
