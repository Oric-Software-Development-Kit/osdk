# osdk/main/tap2dsk/CMakeLists.txt

add_executable(tap2dsk sources/tap2dsk.c)

target_include_directories(tap2dsk PRIVATE includes)

target_link_libraries(tap2dsk PRIVATE common)

install(TARGETS tap2dsk DESTINATION bin)
