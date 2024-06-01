#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/global.h"
#include "engine/utils.h"
#include "engine/config.h"
#include "engine/time.h"
#include "engine/input/input.h"
#include "engine/renderer/renderer.h"
#include "engine/physics/physics.h"

static void handle_input(void);
static bool app_running = true;
static vec2 mouse_pos = {25, 25};

int main(void) {
    if(!glfwInit()) {ERROR_RETURN(1, "Unable to initialize GLFW\n");}

    time_init(60);
    render_init();
    config_init();
    physics_init();
    glfwSetKeyCallback(rendering_state.window, glfw_key_callback);

    AABB test = {
        .pos = {rendering_state.width * 0.5, rendering_state.height * 0.5},
        .half_size = {50, 50}
    };


    while(app_running) {
        time_update();
        handle_input();
        physics_update();
        render_begin();

        render_aabb(&test, (vec4){1, 1, 1, 0.5});

        if (physics_point_intersect(mouse_pos, &test))
            render_quad(mouse_pos, (vec2){10, 10}, (vec4){1, 0, 1, 1});
        else
            render_quad(mouse_pos, (vec2){10, 10}, (vec4){1, 1, 1, 1});

        render_end();
        time_update_end();
    }
    render_exit();

    glfwDestroyWindow(rendering_state.window);
    glfwTerminate();
    return 0;
}

static void handle_input(void) {
    if (keys.escape != KEY_UNPRESSED) app_running = false;
    float64 x, y;
    glfwGetCursorPos(rendering_state.window, &x, &y);
    mouse_pos[0] = (float32) x;
    mouse_pos[1] = (float32) (rendering_state.height - y);
}
