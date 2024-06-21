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
    AABB cursor_aabb = {
        .half_size = {75, 75}
    };
    AABB sum_aabb = {
        .pos = {test.pos[0], test.pos[1]},
        .half_size = {
            test.half_size[0] + cursor_aabb.half_size[0],
            test.half_size[1] + cursor_aabb.half_size[1]
        }
    };


    while(app_running) {
        time_update();
        handle_input();
        physics_update();

        cursor_aabb.pos[0] = mouse_pos[0];  
        cursor_aabb.pos[1] = mouse_pos[1];  

        render_begin();

        render_aabb(&test, (vec4){1, 1, 1, 0.5});
        render_aabb(&sum_aabb, (vec4){1, 1, 1, 0.5});

        AABB minkowsky_diff = minkowsky_diff_aabb(&test, &cursor_aabb);
        render_aabb(&minkowsky_diff, (vec4){0, 1, 1, 0.5});

        vec2 pen_vector;
        minkowsky_diff_pen_vector(pen_vector, &minkowsky_diff);
        AABB collision_aabb = cursor_aabb;
        collision_aabb.pos[0] += pen_vector[0];
        collision_aabb.pos[1] += pen_vector[1];

        if (physics_aabb_intersect(&cursor_aabb, &test)) {
            render_aabb(&cursor_aabb, (vec4){1, 1, 0, 0.5});
            render_aabb(&collision_aabb, (vec4){0, 1, 1, 0.5});
            vec2 line;
            vec2_add(line, cursor_aabb.pos, pen_vector);
            render_line_segment(cursor_aabb.pos, line, (vec4){1, 1, 1, 0.5});
        }
        else
            render_aabb(&cursor_aabb, (vec4){1, 0, 1, 0.5});


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
