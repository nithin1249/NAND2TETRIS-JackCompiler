cmake_minimum_required(VERSION 3.15)
project(NAND2TETRIS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Ensure strictly standard C++)


file(GLOB_RECURSE SOURCES
        "Compiler/*.cpp"
        "Compiler/*.h"
)

#[[ add_executable(NAND2TETRIS ${SOURCES}
        Compiler/Parser/ASTVisitor.h
        Compiler/Parser/PrattParser.cpp
        Compiler/Parser/PrattParser.h)]]

add_executable(parser
        Compiler/Tokenizer/Tokenizer.h
        Compiler/Tokenizer/Tokenizer.cpp
        Compiler/Tokenizer/TokenTypes.h
        Compiler/AST/AST.h
        Compiler/Parser/PrattParser.h
        Compiler/Parser/PrattParser.cpp
        test/jack_parser.cpp
        Compiler/Parser/XMLVisitor.h
        Compiler/Parser/XMLExporter.cpp
        Compiler/Parser/XMLExporter.h
        Compiler/GlobalRegistry/GlobalRegistry.h
        Compiler/GlobalRegistry/GlobalRegistry.cpp
        Compiler/TypeRegistry/TypeRegistry.cpp
        Compiler/TypeRegistry/TypeRegistry.h)

if(WIN32)
    target_link_libraries(NAND2TETRIS psapi)
endif()

#[[ if(APPLE)
    # On Mac -> produces "jack_mac"
    set_target_properties(NAND2TETRIS PROPERTIES OUTPUT_NAME "NAND2TETRIS_mac")
elseif(WIN32)
    # On Windows -> produces "jack_win.exe" (CMake adds .exe automatically)
    set_target_properties(NAND2TETRIS PROPERTIES OUTPUT_NAME "NAND2TETRIS_win")
elseif(UNIX)
    # On Linux -> produces "jack_linux"
    set_target_properties(NAND2TETRIS PROPERTIES OUTPUT_NAME "NAND2TETRIS_linux")
endif()]]

#[[
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
        RENAME README.txt)]]





