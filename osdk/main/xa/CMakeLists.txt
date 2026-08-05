# osdk/main/xa/CMakeLists.txt

add_executable(xa
    sources/xaa.cpp
    sources/xa.cpp
    sources/xal.cpp
    sources/xam.cpp
    sources/xao.cpp
    sources/xap.cpp
    sources/xar2.cpp
    sources/xar.cpp
    sources/xat.cpp
    sources/xau.cpp
)

target_include_directories(xa PRIVATE includes)

target_link_libraries(xa PRIVATE common)

install(TARGETS xa DESTINATION bin)
