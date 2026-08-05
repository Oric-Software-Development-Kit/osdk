# osdk/main/bin2txt/CMakeLists.txt

add_executable(bin2txt sources/bin2txt.cpp)

target_include_directories(bin2txt PRIVATE includes)

target_link_libraries(bin2txt PRIVATE common)

install(TARGETS bin2txt DESTINATION bin)
