cmake_minimum_required(VERSION 3.15)
project(zero_page_code NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME ZPCODE ADDRESS "$600" SOURCES main.s LINK_FLAGS -B)
