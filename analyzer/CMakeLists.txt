file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)
set(GENERATED_SOURCES
    ${GENERATED_DIR}/syntax_parser.cpp
    ${GENERATED_DIR}/lexical_parser.cpp
)
set_source_files_properties(${GENERATED_SOURCES} PROPERTIES GENERATED TRUE)

file(GLOB_RECURSE ANALYZER_SOURCES ${ANALYZER_DIR}/*.cpp)
list(FILTER ANALYZER_SOURCES EXCLUDE REGEX ".*/ir/ir_selftest\\.cpp$")

add_executable(analyzer ${ANALYZER_SOURCES} ${COMMON_SOURCES} ${GENERATED_SOURCES})

if (MSVC)
    target_compile_options(analyzer PRIVATE /bigobj /utf-8)
    target_link_options(analyzer PRIVATE /STACK:8388608)
endif()

add_dependencies(analyzer generate_syntax generate_lex)

target_include_directories(analyzer PRIVATE ${GENERATED_DIR})
target_include_directories(analyzer PRIVATE ${COMMON_DIR})
