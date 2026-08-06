# osdk/main/pictconv/CMakeLists.txt

file(GLOB PICTCONV_SOURCES "sources/*.cpp")
list(FILTER PICTCONV_SOURCES EXCLUDE REGEX ".*video\\.cpp$")

add_executable(pictconv ${PICTCONV_SOURCES})

target_include_directories(pictconv PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/sources
    ${CMAKE_SOURCE_DIR}/shared_libraries/nanosvg/src
    ${CMAKE_SOURCE_DIR}/shared_libraries/freeimage/v3.12.0
    ${CMAKE_SOURCE_DIR}/shared_libraries/freeimage/v3.12.0/Source
)

target_compile_definitions(pictconv PRIVATE FREEIMAGE_LIB)

target_link_libraries(pictconv PRIVATE common FreeImage::FreeImage)

if(NOT WIN32)
    find_library(MATH_LIBRARY m)
    if(MATH_LIBRARY)
        target_link_libraries(pictconv PRIVATE ${MATH_LIBRARY})
    endif()
endif()

install(TARGETS pictconv DESTINATION bin)
