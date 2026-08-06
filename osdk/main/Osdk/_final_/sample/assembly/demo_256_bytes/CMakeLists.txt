cmake_minimum_required(VERSION 3.15)
project(demo_256_bytes NONE)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OSDKSample.cmake")
osdk_add_sample(NAME 256BDEMO TAP_NAME 256BCIRCLES ADDRESS "$600" SOURCES main.s LINK_FLAGS -B)
