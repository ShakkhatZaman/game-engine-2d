cmake_minimum_required(VERSION 3.14)

project(2D_game_engine)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES
    "./src/main.c"
    "./src/vendor/glad.c"
    "./src/engine/global.c"
    "./src/engine/renderer/renderer.c"
    "./src/engine/io/io.c"
    "./src/engine/renderer/renderer_utils.c"
)

set(LINKING_LIBRARIES
    glfw3
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE "./src/include/")
target_link_directories(${PROJECT_NAME} PRIVATE "./src/lib/")

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINKING_LIBRARIES})
