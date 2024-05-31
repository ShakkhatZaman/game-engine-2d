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

static bool app_running = true;

int main(void) {
    if(!glfwInit()) {ERROR_RETURN(1, "Unable to initialize GLFW\n");}

    time_init(60);
    render_init();
    config_init();
    physics_init();

    uint32 body_count = 100;
    for (uint32 i = 0; i < body_count; i++) {
        uint64 body_index = physics_body_create(
            (vec2){rand() % rendering_state.width, rand() % rendering_state.height},
            (vec2){rand() % 100, rand() % 200}
        );
        Body *body = physics_body_get(body_index);
        body->acceleration[0] = (float32)(rand() % 200 - 100);
        body->acceleration[1] = (float32)(rand() % 200 - 100);
    }

    glfwSetKeyCallback(rendering_state.window, glfw_key_callback);
    float32 x = 25.0, y = 25.0;

    while(app_running) {
        time_update();
        physics_update();
        render_begin();

        render_quad(
            (vec2) {x, y},
            (vec2) {50, 50},
            (vec4) {1.0, 0.0, 0.0, 1.0}
        );

        for (uint32 i = 0; i < body_count; i++) {
            Body *body = physics_body_get(i);
            render_quad(body->aabb.pos, body->aabb.half_size, (vec4){1, 1, 0, 1});
            if (body->aabb.pos[0] > rendering_state.width || body->aabb.pos[0] < 0)
                body->velocity[0] *= -1;
            if (body->aabb.pos[1] > rendering_state.width || body->aabb.pos[1] < 0)
                body->velocity[1] *= -1;

            if (body->velocity[0] > 200)
                body->velocity[0] = 200;
            if (body->velocity[0] < -200)
                body->velocity[0] = -200;
            if (body->velocity[1] > 200)
                body->velocity[1] = 200;
            if (body->velocity[1] < -200)
                body->velocity[1] = -200;
        }

        render_end();
        if (keys.right != KEY_UNPRESSED) x += 150 * timing.delta;
        if (keys.left != KEY_UNPRESSED) x -= 150 * timing.delta;
        if (keys.up != KEY_UNPRESSED) y += 150 * timing.delta;
        if (keys.down != KEY_UNPRESSED) y -= 150 * timing.delta;
        if (keys.escape != KEY_UNPRESSED) app_running = false;
        time_update_end();
    /* printf("%d\n", timing.frame_rate); */
    }
    render_exit();

    glfwDestroyWindow(rendering_state.window);
    glfwTerminate();
    printf("%d\n", timing.frame_count);
    printf("%f\n", timing.frame_time);
    printf("%f\n", timing.frame_delay);
    printf("%f\n", timing.delta);
    return 0;
}
