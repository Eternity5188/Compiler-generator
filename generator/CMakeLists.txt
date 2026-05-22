file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

set(LEXICAL_SOURCES
    ${GNERATOR_DIR}/lexical/lexical_generator.cpp
    ${GNERATOR_DIR}/lexical/ReadLex.cpp
    ${GNERATOR_DIR}/lexical/NormaliseRE.cpp
    ${GNERATOR_DIR}/lexical/Infix2Postfix.cpp
    ${GNERATOR_DIR}/lexical/NFA.cpp
    ${GNERATOR_DIR}/lexical/DFA.cpp
    ${GNERATOR_DIR}/lexical/GenCode.cpp
)

set(SYNTAX_SOURCES
    ${GNERATOR_DIR}/syntax/syntax_generator.cpp
    ${GNERATOR_DIR}/syntax/syntax_rule_parser.cpp
    ${GNERATOR_DIR}/syntax/production.cpp
    ${GNERATOR_DIR}/syntax/symbol.cpp
    ${GNERATOR_DIR}/syntax/grammar.cpp
    ${GNERATOR_DIR}/syntax/action.cpp
    ${GNERATOR_DIR}/syntax/lr_item.cpp
    ${GNERATOR_DIR}/syntax/lr_state.cpp
    ${GNERATOR_DIR}/syntax/lr_parser.cpp
    ${GNERATOR_DIR}/syntax/ast_node.cpp
    ${GNERATOR_DIR}/syntax/ast_tree.cpp
)

add_executable(lexical_generator ${LEXICAL_SOURCES} ${COMMON_SOURCES})
add_executable(syntax_generator ${SYNTAX_SOURCES} ${COMMON_SOURCES})

target_include_directories(lexical_generator PRIVATE ${COMMON_DIR} ${GNERATOR_DIR}/lexical)
target_include_directories(syntax_generator PRIVATE ${COMMON_DIR} ${GNERATOR_DIR}/syntax)

add_custom_command(
    OUTPUT ${GENERATED_DIR}/lexical_parser.cpp ${GENERATED_DIR}/lexical_parser.h
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
    COMMAND $<TARGET_FILE:lexical_generator>
            ${GNERATOR_DIR}/lexical/c99.l
            ${GENERATED_DIR}/lexical_parser.cpp
            ${GENERATED_DIR}/lexical_parser.h
    DEPENDS lexical_generator ${GNERATOR_DIR}/lexical/c99.l
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running lexical_generator..."
    VERBATIM
)
add_custom_target(generate_lex ALL
    DEPENDS ${GENERATED_DIR}/lexical_parser.cpp ${GENERATED_DIR}/lexical_parser.h
)

add_custom_command(
    OUTPUT ${GENERATED_DIR}/syntax_parser.cpp ${GENERATED_DIR}/syntax_parser.h
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
    COMMAND $<TARGET_FILE:syntax_generator>
    DEPENDS syntax_generator generate_lex ${CMAKE_SOURCE_DIR}/resource/rule/syntax_rule.txt
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running syntax_generator..."
    VERBATIM
)
add_custom_target(generate_syntax ALL
    DEPENDS ${GENERATED_DIR}/syntax_parser.cpp ${GENERATED_DIR}/syntax_parser.h
)
