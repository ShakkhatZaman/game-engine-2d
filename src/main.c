#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/global.h"
#include "engine/renderer/renderer.h"
#include "engine/utils.h"
#include "engine/input.h"
#include "engine/config.h"
#include "engine/time.h"

static bool app_running = true;

int main(void) {
    if(!glfwInit()) {ERROR_RETURN(1, "Unable to initialize GLFW\n");}

    render_init();
    config_init();
    time_init(60);

    glfwSetKeyCallback(rendering_state.window, glfw_key_callback);
    float32 x = 25.0, y = 25.0;

    while(app_running) {
        time_update();
        render_begin();

        render_quad(
            (vec2) {x, y},
            (vec2) {50, 50},
            (vec4) {1.0, 0.0, 0.0, 1.0}
        );

        render_end();
        if (keys.right != KEY_UNPRESSED) x += 150 * timing.delta;
        if (keys.left != KEY_UNPRESSED) x -= 150 * timing.delta;
        if (keys.up != KEY_UNPRESSED) y += 150 * timing.delta;
        if (keys.down != KEY_UNPRESSED) y -= 150 * timing.delta;
        if (keys.escape != KEY_UNPRESSED) app_running = false;
        time_update_end();
    }
    render_exit();

    glfwDestroyWindow(rendering_state.window);
    glfwTerminate();
    return 0;
}
