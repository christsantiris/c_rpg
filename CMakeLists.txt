cmake_minimum_required(VERSION 3.20)
project(conr C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_C_FLAGS_Release "-O2")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/*.c
    ${CMAKE_SOURCE_DIR}/src/game/*.c
    ${CMAKE_SOURCE_DIR}/src/renderer/viewport.c
)

add_executable(conr ${SOURCES})
target_include_directories(conr PRIVATE src ${SDL2_INCLUDE_DIRS})
target_link_libraries(conr PRIVATE ${SDL2_LIBRARIES})

add_executable(test_runner ${TEST_SOURCES})
target_include_directories(test_runner PRIVATE src)

add_custom_target(run
    COMMAND ./conr
    DEPENDS conr
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(test
    COMMAND ./test_runner
    DEPENDS test_runner
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)