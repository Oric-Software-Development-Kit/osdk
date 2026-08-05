# euphoric_tools/tap2dsk/CMakeLists.txt

add_executable(euphoric_tap2dsk tap2dsk.c)
set_target_properties(euphoric_tap2dsk PROPERTIES OUTPUT_NAME "tap2dsk")
target_include_directories(euphoric_tap2dsk PRIVATE ${CMAKE_SOURCE_DIR}/osdk/main/tap2dsk/sources)

install(TARGETS euphoric_tap2dsk DESTINATION bin)
