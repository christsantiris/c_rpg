cmake_minimum_required(VERSION 3.20)
project(conr C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_C_FLAGS_Release "-O2")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(EMSCRIPTEN)
    # Browser dependencies come from Emscripten ports, not host SDL libraries.
elseif(APPLE)
    find_package(SDL2 REQUIRED)
    find_library(SDL2_TTF_LIBRARIES SDL2_ttf REQUIRED)
    find_path(SDL2_TTF_INCLUDE_DIRS SDL2/SDL_ttf.h REQUIRED)
    find_library(SDL2_MIXER_LIBRARIES SDL2_mixer REQUIRED)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
endif()

file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/external/*.c
)
file(GLOB_RECURSE TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/*.c
    ${CMAKE_SOURCE_DIR}/src/game/*.c
    ${CMAKE_SOURCE_DIR}/src/renderer/viewport.c
)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/screens/shop.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/screens/workshop.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/screens/harbor.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/screens/slot_select.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/screens/quest_journal.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/screens/inventory.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/screens/controls_screen.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/screens/landing.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/systems/save_load.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/systems/controls_settings.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/src/systems/highscore.c)
list(APPEND TEST_SOURCES ${CMAKE_SOURCE_DIR}/external/cJSON.c)

add_executable(conr ${SOURCES})
target_include_directories(conr PRIVATE src ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/external)
target_link_libraries(conr PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES})

if(EMSCRIPTEN)
    target_compile_options(conr PRIVATE -sUSE_SDL=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2)
    target_link_options(conr PRIVATE
        -sUSE_SDL=2
        -sUSE_SDL_TTF=2
        -sUSE_SDL_MIXER=2
        "-sSDL2_MIXER_FORMATS=['mp3','ogg']"
        -sASYNCIFY=1
        -sASYNCIFY_STACK_SIZE=131072
        -sSTACK_SIZE=8388608
        -sALLOW_MEMORY_GROWTH=1
        -sENVIRONMENT=web
        "-sEXPORTED_RUNTIME_METHODS=['callMain','FS','IDBFS','addRunDependency','removeRunDependency']"
        -lidbfs.js
        --preload-file "${CMAKE_SOURCE_DIR}/assets@/assets"
        --shell-file "${CMAKE_SOURCE_DIR}/package/browser/shell.html"
    )
    set_target_properties(conr PROPERTIES
        OUTPUT_NAME index
        SUFFIX .html
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/browser"
        LINK_DEPENDS "${CMAKE_SOURCE_DIR}/package/browser/shell.html"
    )
endif()

if(NOT EMSCRIPTEN)
add_executable(test_runner ${TEST_SOURCES})
target_include_directories(test_runner PRIVATE src ${CMAKE_SOURCE_DIR}/external)
target_compile_definitions(test_runner PRIVATE TEST_BUILD)

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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_SHORTCUT_TEST_SOURCES ${SOURCES})
    list(REMOVE_ITEM DEBUG_SHORTCUT_TEST_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")
    add_executable(debug_shortcuts_test EXCLUDE_FROM_ALL
        ${DEBUG_SHORTCUT_TEST_SOURCES}
        ${CMAKE_SOURCE_DIR}/tools/tests/debug_shortcuts_test.c
    )
    target_include_directories(debug_shortcuts_test PRIVATE src ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/external)
    target_link_libraries(debug_shortcuts_test PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES})
    add_custom_target(test-debug-shortcuts
        COMMAND $<TARGET_FILE:debug_shortcuts_test>
        DEPENDS debug_shortcuts_test
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
endif()
