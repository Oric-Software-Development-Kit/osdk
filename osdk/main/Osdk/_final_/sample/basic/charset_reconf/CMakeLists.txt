cmake_minimum_required(VERSION 3.15)
project(charset_reconf NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME CHARDEF SOURCES main.bas font.bas)
