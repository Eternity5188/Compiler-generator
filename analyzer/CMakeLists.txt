file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

set(GENERATED_SOURCES
    ${GENERATED_DIR}/syntax_parser.cpp
    ${GENERATED_DIR}/lexical_parser.cpp
)

file(GLOB_RECURSE ANALYZER_SOURCES ${ANALYZER_DIR}/*.cpp)
set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/ir/ir_unit_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/system/system_integration_tests.cpp
)

add_executable(analyzer ${ANALYZER_SOURCES} ${TEST_SOURCES} ${COMMON_SOURCES} ${GENERATED_SOURCES})

if (MSVC)
    target_compile_options(analyzer PRIVATE /bigobj /utf-8)
    target_link_options(analyzer PRIVATE /STACK:8388608)
endif()

add_dependencies(analyzer generate_syntax generate_lex)

target_include_directories(analyzer PRIVATE ${COMMON_DIR} ${GENERATED_DIR} ${ANALYZER_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../tests)
