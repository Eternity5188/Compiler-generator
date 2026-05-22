cmake_minimum_required(VERSION 3.15)
project(c99lex VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

set(SOURCES
    lexical_generator.cpp
    ReadLex.cpp
    NormaliseRE.cpp
    Infix2Postfix.cpp
    NFA.cpp
    DFA.cpp
    GenCode.cpp
)

add_executable(lexical_generator ${SOURCES})
