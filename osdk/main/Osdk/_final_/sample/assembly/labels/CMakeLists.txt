cmake_minimum_required(VERSION 3.15)
project(labels_assembly NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME LABELS ADDRESS "$600" SOURCES main.s LINK_FLAGS -B XA_FLAGS -a)
