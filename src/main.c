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

AABB start_aabb = {
    .half_size = {75, 75}
};

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
        render_aabb(&start_aabb, (vec4){0, 0, 1, 0.5});
        if (physics_aabb_intersect(&cursor_aabb, &test)) {
            render_aabb(&cursor_aabb, (vec4){1, 1, 0, 0.5});
        }
        else
            render_aabb(&cursor_aabb, (vec4){1, 0, 1, 0.5});

        vec4 faded = {1, 1, 1, 0.3};
        render_line_segment(start_aabb.pos, mouse_pos, faded);
        float32 x = sum_aabb.pos[0], y = sum_aabb.pos[1];
        render_line_segment((vec2){x - sum_aabb.half_size[0], 0}, (vec2){x - sum_aabb.half_size[0], rendering_state.height}, faded);
        render_line_segment((vec2){x + sum_aabb.half_size[0], 0}, (vec2){x + sum_aabb.half_size[0], rendering_state.height}, faded);
        render_line_segment((vec2){0, y - sum_aabb.half_size[1]}, (vec2){rendering_state.width, y - sum_aabb.half_size[1]}, faded);
        render_line_segment((vec2){0, y + sum_aabb.half_size[1]}, (vec2){rendering_state.width, y + sum_aabb.half_size[1]}, faded);
        
        vec2 min, max, magnitude;
        aabb_min_max(min, max, &sum_aabb);
        vec2_sub(magnitude, mouse_pos, start_aabb.pos);
        Collision result = ray_collide_aabb(start_aabb.pos, magnitude, sum_aabb);

        if (result.collided) {
            AABB hit_aabb = {
                .pos = {result.pos[0], result.pos[1]},
                .half_size = {start_aabb.half_size[0], start_aabb.half_size[1]}
            };
            render_aabb(&hit_aabb, (vec4){0.3, 0.3, 1, 1});
            render_quad(result.pos, (vec2){5, 5}, (vec4){0.3, 0.3, 1, 1});
        }


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

    int mouse_left = glfwGetMouseButton(rendering_state.window, GLFW_MOUSE_BUTTON_LEFT);
    if (mouse_left == GLFW_PRESS) {
        start_aabb.pos[0] = mouse_pos[0];
        start_aabb.pos[1] = mouse_pos[1];
    }
}
