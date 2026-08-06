# osdk/main/CMakeLists.txt
# =============================================
# OSDK core library and command line tools
# =============================================

add_subdirectory(common)

add_subdirectory(bas2tap)
add_subdirectory(bin2txt)
add_subdirectory(compiler)
add_subdirectory(DskTool)
add_subdirectory(filepack)
add_subdirectory(FloppyBuilder)
add_subdirectory(header)
add_subdirectory(link65)
add_subdirectory(macrosplitter)
add_subdirectory(MemMap)
add_subdirectory(old2mfm)
add_subdirectory(opt65)
add_subdirectory(pictconv)
add_subdirectory(tap2cd)
add_subdirectory(tap2dsk)
add_subdirectory(TapTool)
add_subdirectory(xa)
add_subdirectory(Ym2Mym)

# Install OSDK data directories and files if present
set(OSDK_FINAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Osdk/_final_")
if(EXISTS "${OSDK_FINAL_DIR}")
    install(DIRECTORY "${OSDK_FINAL_DIR}/documentation" DESTINATION .)
    install(DIRECTORY "${OSDK_FINAL_DIR}/include" DESTINATION .)
    install(DIRECTORY "${OSDK_FINAL_DIR}/lib" DESTINATION .)
    install(DIRECTORY "${OSDK_FINAL_DIR}/macro" DESTINATION .)
    install(DIRECTORY "${OSDK_FINAL_DIR}/sample" DESTINATION .)
    install(DIRECTORY "${OSDK_FINAL_DIR}/TMP" DESTINATION .)
    install(DIRECTORY "${OSDK_FINAL_DIR}/Oricutron" DESTINATION .)
    install(DIRECTORY "${OSDK_FINAL_DIR}/Roms" DESTINATION Oricutron)
    install(FILES "${OSDK_FINAL_DIR}/read me.txt" DESTINATION .)

    # Create directory symlinks and lowercase binary symlinks
    install(CODE [[
        message(STATUS "Creating OSDK symlinks in ${CMAKE_INSTALL_PREFIX}...")
        if(UNIX)
            execute_process(COMMAND ln -sfn bin "${CMAKE_INSTALL_PREFIX}/Bin" ERROR_QUIET)
            # macOS filesystems are commonly case-insensitive: creating this
            # compatibility link there replaces MACROS.H with a self-link.
            if(NOT APPLE)
                execute_process(COMMAND ln -sfn MACROS.H "${CMAKE_INSTALL_PREFIX}/macro/macros.h" ERROR_QUIET)
            endif()
            execute_process(COMMAND ln -sfn Roms "${CMAKE_INSTALL_PREFIX}/roms" ERROR_QUIET)
            execute_process(COMMAND ln -sfn TMP "${CMAKE_INSTALL_PREFIX}/tmp" ERROR_QUIET)
        else()
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink bin "${CMAKE_INSTALL_PREFIX}/Bin")
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink MACROS.H "${CMAKE_INSTALL_PREFIX}/macro/macros.h")
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink Roms "${CMAKE_INSTALL_PREFIX}/roms")
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink TMP "${CMAKE_INSTALL_PREFIX}/tmp")
        endif()

        # Case-only aliases are self-links on the usual case-insensitive
        # macOS filesystems, so retain the canonical executable names there.
        if(NOT APPLE)
            foreach(B Bin2Tap DskTool FloppyBuilder TapTool Ym2Mym)
                string(TOLOWER "${B}" b)
                if(EXISTS "${CMAKE_INSTALL_PREFIX}/bin/${B}" AND NOT "${B}" STREQUAL "${b}")
                    if(UNIX)
                        execute_process(COMMAND ln -sfn "${B}" "${CMAKE_INSTALL_PREFIX}/bin/${b}" ERROR_QUIET)
                    else()
                        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${B}" "${CMAKE_INSTALL_PREFIX}/bin/${b}")
                    endif()
                endif()
            endforeach()
        endif()
    ]])
endif()
