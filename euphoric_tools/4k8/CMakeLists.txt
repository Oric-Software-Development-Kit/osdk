# euphoric_tools/4k8/CMakeLists.txt

add_executable(4k8towav 4k8towav.c)
add_executable(4k8totap 4k8totap.c)

install(TARGETS 4k8towav 4k8totap DESTINATION bin)
