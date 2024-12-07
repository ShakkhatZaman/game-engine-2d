cmake_minimum_required(VERSION 3.14)

project(2D_game_engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RELEASE_BUILD "Building for release" OFF)

set(SOURCE_FILES
    "./src/main.c"
    "./src/vendor/glad.c"
    "./src/engine/global.c"
    "./src/engine/time.c"
    "./src/engine/config.c"
    "./src/engine/list.c"
    "./src/engine/weapons.c"
    "./src/engine/audio/audio.c"
    "./src/engine/animation/animation.c"
    "./src/engine/entities/entities.c"
    "./src/engine/io/io.c"
    "./src/engine/input/input.c"
    "./src/engine/physics/physics.c"
    "./src/engine/renderer/renderer.c"
    "./src/engine/renderer/renderer_utils.c"
    "./src/engine/renderer/renderer_internal.c"
)

set(LINKING_LIBRARIES
    SDL2
    SDL2_mixer
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if(RELEASE_BUILD)
    set(CMAKE_BUILD_TYPE Release)
else()
    set(CMAKE_BUILD_TYPE Debug)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _DEBUG_)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "./src/include/")
target_link_directories(${PROJECT_NAME} PRIVATE "./lib/")

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINKING_LIBRARIES})

unset(RELEASE_BUILD CACHE)
