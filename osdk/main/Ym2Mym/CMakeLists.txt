# osdk/main/Ym2Mym/CMakeLists.txt

add_executable(Ym2Mym
    sources/lzhlib.cpp
    sources/Ym2Mym.cpp
)

target_include_directories(Ym2Mym PRIVATE includes)

target_link_libraries(Ym2Mym PRIVATE common)

install(TARGETS Ym2Mym DESTINATION bin)
