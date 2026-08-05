# osdk/main/opt65/CMakeLists.txt

add_executable(opt65 sources/opt65.c)

target_include_directories(opt65 PRIVATE includes)

target_link_libraries(opt65 PRIVATE common)

install(TARGETS opt65 DESTINATION bin)
