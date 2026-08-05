cmake_minimum_required(VERSION 3.15)
project(hires_draw NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME HIRESDRAW ADDRESS "$800" SOURCES main.c)
