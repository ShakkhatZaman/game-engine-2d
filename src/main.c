#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/global.h"
#include "engine/renderer/renderer.h"
#include "engine/utils.h"

int main(void) {
    bool app_running = true;
    if(!glfwInit()) {ERROR_RETURN(1, "Unable to initialize GLFW\n");}

    render_init();

    while(app_running) {
        app_running = !glfwWindowShouldClose(rendering_state.window);
        if (glfwGetKey(rendering_state.window, GLFW_KEY_ESCAPE)) app_running = false;

        render_begin();

        render_quad(
            (vec2) {25.0, 25.0},
            (vec2) {50, 50},
            (vec4) {1.0, 0.0, 0.0, 1.0}
        );

        render_end();
    }

    glfwDestroyWindow(rendering_state.window);
    glfwTerminate();
    return 0;
}
