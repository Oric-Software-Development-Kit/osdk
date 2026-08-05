# osdk/main/TapTool/CMakeLists.txt

add_executable(TapTool sources/TapTool.cpp)

target_include_directories(TapTool PRIVATE includes)

target_link_libraries(TapTool PRIVATE common)

install(TARGETS TapTool DESTINATION bin)
