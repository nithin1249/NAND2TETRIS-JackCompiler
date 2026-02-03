cmake_minimum_required(VERSION 3.15)
project(NAND2TETRIS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Ensure strictly standard C++)


file(GLOB_RECURSE SOURCES
        "Compiler/*.cpp"
        "Compiler/*.h"
)

add_executable(NAND2TETRIS ${SOURCES})

if(WIN32)
    target_link_libraries(NAND2TETRIS psapi)
endif()

if(APPLE)
    # On Mac -> produces "jack_mac"
    set_target_properties(NAND2TETRIS PROPERTIES OUTPUT_NAME "NAND2TETRIS_mac")
elseif(WIN32)
    # On Windows -> produces "jack_win.exe" (CMake adds .exe automatically)
    set_target_properties(NAND2TETRIS PROPERTIES OUTPUT_NAME "NAND2TETRIS_win.exe")
elseif(UNIX)
    # On Linux -> produces "jack_linux"
    set_target_properties(NAND2TETRIS PROPERTIES OUTPUT_NAME "NAND2TETRIS_linux")
endif()

install(TARGETS NAND2TETRIS DESTINATION bin)
install(DIRECTORY os DESTINATION .)
install(DIRECTORY tools DESTINATION .)
install(DIRECTORY JackCode DESTINATION .)
if(WIN32)
    # Windows only needs the batch file
    install(FILES scripts/install.bat DESTINATION .)

elseif(UNIX)
    # Mac & Linux (UNIX) only need the shell script
    install(PROGRAMS scripts/install.sh DESTINATION .)

endif()
install(FILES docs/RELEASE_README.txt
        DESTINATION .
        RENAME README.txt)





