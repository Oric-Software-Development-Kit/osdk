# osdk/main/header/CMakeLists.txt

add_executable(header sources/header.cpp)

target_include_directories(header PRIVATE includes)

target_link_libraries(header PRIVATE common)

install(TARGETS header DESTINATION bin)
