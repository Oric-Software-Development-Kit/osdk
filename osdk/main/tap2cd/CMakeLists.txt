# osdk/main/tap2cd/CMakeLists.txt

add_executable(tap2cd sources/tap2cd.c)

target_include_directories(tap2cd PRIVATE includes)

target_link_libraries(tap2cd PRIVATE common)

install(TARGETS tap2cd DESTINATION bin)
