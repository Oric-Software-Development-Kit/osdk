cmake_minimum_required(VERSION 3.15)
project(italicize NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME ITALICS ADDRESS "$8000" SOURCES italics.s main.bas LINK_FLAGS -B)
