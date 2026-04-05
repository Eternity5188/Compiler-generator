file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

file(GLOB LEXICAL_SOURCES ${GNERATOR_DIR}/lexical/*.cpp)
file(GLOB SYNTAX_SOURCES ${GNERATOR_DIR}/syntax/*.cpp)
file(GLOB INTERMEDIATE_SOURCES ${GNERATOR_DIR}/intermediate/*.cpp)

add_executable(lexical_generator ${LEXICAL_SOURCES} ${COMMON_SOURCES})
add_executable(syntax_generator ${SYNTAX_SOURCES} ${COMMON_SOURCES})
add_executable(intermediate_generator ${INTERMEDIATE_SOURCES} ${COMMON_SOURCES})

target_include_directories(lexical_generator PRIVATE ${COMMON_DIR})
target_include_directories(syntax_generator PRIVATE ${COMMON_DIR})
target_include_directories(intermediate_generator PRIVATE ${COMMON_DIR})