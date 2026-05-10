cmake_minimum_required(VERSION 3.21)

project(compiler_generator)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(GNERATOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generator)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resource/source)
set(ANALYZER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/analyzer)
set(TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/target)

add_subdirectory(${GNERATOR_DIR} ${TARGET_DIR}/generator)
add_subdirectory(${ANALYZER_DIR} ${TARGET_DIR}/analyzer)