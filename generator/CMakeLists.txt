file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

set(LEXICAL_SOURCES lexical_generator.cpp)
set(SYNTAX_SOURCES syntax_generator.cpp)

add_executable(lexical_generator ${LEXICAL_SOURCES} ${COMMON_SOURCES})
add_executable(syntax_generator ${SYNTAX_SOURCES} ${COMMON_SOURCES})

target_include_directories(lexical_generator PRIVATE ${COMMON_DIR})
target_include_directories(syntax_generator PRIVATE ${COMMON_DIR})