file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

file(GLOB_RECURSE LEXICAL_SOURCES ${GNERATOR_DIR}/lexical/*.cpp)
file(GLOB_RECURSE SYNTAX_SOURCES ${GNERATOR_DIR}/syntax/*.cpp)

add_executable(lexical_generator ${LEXICAL_SOURCES} ${COMMON_SOURCES})
add_executable(syntax_generator ${SYNTAX_SOURCES} ${COMMON_SOURCES})

target_include_directories(lexical_generator PRIVATE ${COMMON_DIR} ${GNERATOR_DIR}/lexical)
target_include_directories(syntax_generator PRIVATE ${COMMON_DIR} ${GNERATOR_DIR}/syntax)

add_custom_command(
    OUTPUT ${GENERATED_DIR}/lexical_parser.cpp ${GENERATED_DIR}/lexical_parser.h
    COMMAND ${TARGET_DIR}/generator/lexical_generator
    DEPENDS lexical_generator
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running lexical generator..."
    VERBATIM
)
add_custom_target(generate_lex ALL
    DEPENDS ${GENERATED_DIR}/lexical_parser.cpp ${GENERATED_DIR}/lexical_parser.h
)

add_custom_command(
    OUTPUT ${GENERATED_DIR}/syntax_parser.cpp ${GENERATED_DIR}/syntax_parser.h
    COMMAND ${TARGET_DIR}/generator/syntax_generator
    DEPENDS generate_lex syntax_generator
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running syntax generator..."
    VERBATIM
)
add_custom_target(generate_syntax ALL
    DEPENDS ${GENERATED_DIR}/syntax_parser.cpp ${GENERATED_DIR}/syntax_parser.h
)
