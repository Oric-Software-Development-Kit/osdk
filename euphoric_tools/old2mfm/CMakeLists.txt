# euphoric_tools/old2mfm/CMakeLists.txt

add_executable(euphoric_old2mfm old2mfm.c)
set_target_properties(euphoric_old2mfm PROPERTIES OUTPUT_NAME "old2mfm")

install(TARGETS euphoric_old2mfm DESTINATION bin)
