cmake_minimum_required(VERSION 3.20)
project(conr C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_C_FLAGS_Release "-O2")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE SOURCES src/*.c)

add_executable(conr ${SOURCES})

target_include_directories(conr PRIVATE src ${SDL2_INCLUDE_DIRS})
target_link_libraries(conr PRIVATE ${SDL2_LIBRARIES})

add_custom_target(run
    COMMAND ./conr
    DEPENDS conr
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)