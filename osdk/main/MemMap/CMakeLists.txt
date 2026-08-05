# osdk/main/MemMap/CMakeLists.txt

add_executable(memmap sources/memmap.cpp)

target_include_directories(memmap PRIVATE includes)

target_link_libraries(memmap PRIVATE common)

install(TARGETS memmap DESTINATION bin)
