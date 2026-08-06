cmake_minimum_required(VERSION 3.15)
project(game_4kkong NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME Kong ADDRESS "$600" SOURCES main.s spr.s vars.s LINK_FLAGS -B)
