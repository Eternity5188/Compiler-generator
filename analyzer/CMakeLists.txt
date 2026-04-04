file(GLOB_RECURSE COMMON_SOURCES ${COMMON_DIR}/*.cpp)

file(GLOB ANALYZER_SOURCES ${ANALYZER_DIR}/*.cpp)

add_executable(analyzer ${ANALYZER_SOURCES} ${COMMON_SOURCES})

target_include_directories(analyzer PRIVATE ${COMMON_DIR})