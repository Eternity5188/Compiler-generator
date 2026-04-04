file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

set(ANALYZER_SOURCES main.cpp)

add_executable(analyzer ${ANALYZER_SOURCES} ${COMMON_SOURCES})

target_include_directories(analyzer PRIVATE ${COMMON_DIR})

message(STATUS ${COMMON_DIR})