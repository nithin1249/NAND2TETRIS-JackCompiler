cmake_minimum_required(VERSION 4.0)
project(NAND2TETRIS)

set(CMAKE_CXX_STANDARD 26)

add_executable(NAND2TETRIS main.cpp
        Compiler/Tokenizer.cpp
        Compiler/Tokenizer.h
        Compiler/TokenTypes.h
)

add_executable(jack_tokens
        "tests/JackTokensTest/jack_tokens.cpp"
        Compiler/Tokenizer.cpp
        Compiler/Tokenizer.h
        Compiler/TokenTypes.h
)
target_include_directories(jack_tokens PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

