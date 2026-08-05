# osdk/main/filepack/CMakeLists.txt

add_executable(filepack sources/filepack.cpp)

target_include_directories(filepack PRIVATE includes)

target_link_libraries(filepack PRIVATE common)

install(TARGETS filepack DESTINATION bin)
