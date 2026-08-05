# osdk/main/DskTool/CMakeLists.txt

add_executable(DskTool sources/DskTool.cpp)

target_include_directories(DskTool PRIVATE includes)

target_link_libraries(DskTool PRIVATE common)

install(TARGETS DskTool DESTINATION bin)
