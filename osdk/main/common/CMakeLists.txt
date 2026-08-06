# osdk/main/common/CMakeLists.txt

add_library(common STATIC
    sources/common.cpp
    sources/lz_pack.cpp
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/sources
)

if(NOT WIN32)
    target_include_directories(common PUBLIC ${CURSES_INCLUDE_DIRS})
    target_link_libraries(common PUBLIC ${CURSES_LIBRARIES})
endif()

target_compile_definitions(common PRIVATE _LIB)
