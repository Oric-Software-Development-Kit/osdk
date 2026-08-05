# osdk/main/link65/CMakeLists.txt

add_executable(link65 sources/Link65.cpp)

target_include_directories(link65 PRIVATE includes)

target_link_libraries(link65 PRIVATE common)

install(TARGETS link65 DESTINATION bin)
