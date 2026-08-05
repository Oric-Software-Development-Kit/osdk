# osdk/main/macrosplitter/CMakeLists.txt

add_executable(macrosplitter sources/macrosplitter.cpp)

target_include_directories(macrosplitter PRIVATE includes)

target_link_libraries(macrosplitter PRIVATE common)

install(TARGETS macrosplitter DESTINATION bin)
