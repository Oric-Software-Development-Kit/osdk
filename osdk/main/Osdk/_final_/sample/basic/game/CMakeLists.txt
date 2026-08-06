cmake_minimum_required(VERSION 3.15)
project(basic_game NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME Bomber SOURCES BomberZ.bas)
