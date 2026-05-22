file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

file(GLOB SYNTAX_SOURCES ${GNERATOR_DIR}/syntax/*.cpp)
set(LEXICAL_GENERATOR_SOURCES
    ${GNERATOR_DIR}/lexical/lexical_generator.cpp
)

add_executable(lexical_generator ${LEXICAL_GENERATOR_SOURCES} ${COMMON_SOURCES})
add_executable(syntax_generator ${SYNTAX_SOURCES} ${COMMON_SOURCES})

target_include_directories(lexical_generator PRIVATE ${COMMON_DIR})
target_include_directories(syntax_generator PRIVATE ${COMMON_DIR})


# lexical_generator
# add_custom_command(
#     OUTPUT ${GENERATED_DIR}/lexical_parser.cpp ${GENERATED_DIR}/lexical_parser.h  # 你生成的词法文件
#     COMMAND $<TARGET_FILE:lexical_generator>
#     DEPENDS lexical_generator
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#     COMMENT "Running lexical_generator..."
#     VERBATIM
# )
# add_custom_target(generate_lex ALL
#     DEPENDS ${GENERATED_DIR}/lexical_parser.cpp ${GENERATED_DIR}/lexical_parser.h
# )

add_custom_command(
    OUTPUT ${GENERATED_DIR}/syntax_parser.cpp ${GENERATED_DIR}/syntax_parser.h
    COMMAND $<TARGET_FILE:syntax_generator>
    # DEPENDS syntax_generator generate_lex
    DEPENDS syntax_generator
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running syntax_generator..."
    VERBATIM
)
add_custom_target(generate_syntax ALL
    DEPENDS ${GENERATED_DIR}/syntax_parser.cpp ${GENERATED_DIR}/syntax_parser.h
)