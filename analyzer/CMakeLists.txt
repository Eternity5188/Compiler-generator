file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)
set(GENERATED_SOURCES
    ${GENERATED_DIR}/syntax_parser.cpp
)

file(GLOB ANALYZER_SOURCES ${ANALYZER_DIR}/*.cpp)

add_executable(analyzer ${ANALYZER_SOURCES} ${COMMON_SOURCES} ${GENERATED_SOURCES})

add_dependencies(analyzer generate_syntax)

target_include_directories(analyzer PRIVATE ${GENERATED_DIR})
target_include_directories(analyzer PRIVATE ${COMMON_DIR})