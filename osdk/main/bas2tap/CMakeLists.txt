# osdk/main/bas2tap/CMakeLists.txt

add_executable(bas2tap sources/bas2tap.cpp)

target_include_directories(bas2tap PRIVATE includes)

target_link_libraries(bas2tap PRIVATE common)

install(TARGETS bas2tap DESTINATION bin)
