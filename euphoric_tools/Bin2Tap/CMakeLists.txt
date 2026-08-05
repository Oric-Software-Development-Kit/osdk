# euphoric_tools/Bin2Tap/CMakeLists.txt

if(WIN32)
    add_executable(Bin2Tap
        Source/Bin2Tap.cpp
        Source/CommandLine.cpp
        Source/OricBinaryTapeImage.cpp
        Source/PulseWaveWriter.cpp
        Source/TapeWriter.cpp
    )

    target_include_directories(Bin2Tap PRIVATE Source)

    install(TARGETS Bin2Tap DESTINATION bin)
endif()
