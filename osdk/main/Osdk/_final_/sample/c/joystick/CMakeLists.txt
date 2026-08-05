cmake_minimum_required(VERSION 3.15)
project(joystick NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME OsdkJoyTest ADDRESS "$500" SOURCES main.c)
