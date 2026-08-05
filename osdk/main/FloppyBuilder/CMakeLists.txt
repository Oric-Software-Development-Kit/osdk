# osdk/main/FloppyBuilder/CMakeLists.txt

add_executable(FloppyBuilder
    FloppyBuilder.cpp
    Floppy.cpp
)

target_include_directories(FloppyBuilder PRIVATE includes)

target_link_libraries(FloppyBuilder PRIVATE common)

install(TARGETS FloppyBuilder DESTINATION bin)
