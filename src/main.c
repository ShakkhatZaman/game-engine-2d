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
static Body *player_body = NULL;

int main(void) {
    if(!glfwInit()) {ERROR_RETURN(1, "Unable to initialize GLFW\n");}

    time_init(60);
    render_init();
    config_init();
    physics_init();
    glfwSetKeyCallback(rendering_state.window, glfw_key_callback);

    uint32 player_body_id = physics_body_create((vec2){100, 400}, (vec2){50, 50});
    float32 width = rendering_state.width;
    float32 height = rendering_state.height;
    uint32 static_body_a_id = physics_static_body_create((vec2){width * 0.5 - 10, height - 10}, (vec2){width - 20, 20});
    uint32 static_body_b_id = physics_static_body_create((vec2){width - 10, height * 0.5 + 10}, (vec2){20, height - 20});
    uint32 static_body_c_id = physics_static_body_create((vec2){width * 0.5 + 10, 10}, (vec2){width - 20, 20});
    uint32 static_body_d_id = physics_static_body_create((vec2){10, height * 0.5 - 10}, (vec2){20, height - 20});
    uint32 static_body_e_id = physics_static_body_create((vec2){width * 0.5, height * 0.5}, (vec2){50, 50});

    while(app_running) {
        time_update();

        player_body = physics_body_get(player_body_id);
        Static_body *static_body_a = physics_static_body_get(static_body_a_id);
        Static_body *static_body_b = physics_static_body_get(static_body_b_id);
        Static_body *static_body_c = physics_static_body_get(static_body_c_id);
        Static_body *static_body_d = physics_static_body_get(static_body_d_id);
        Static_body *static_body_e = physics_static_body_get(static_body_e_id);

        handle_input();
        physics_update();
        render_begin();

        render_aabb(&player_body->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_a->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_b->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_c->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_d->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_e->aabb, (vec4){1, 1, 1, 1});

        render_end();
        time_update_end();
    }
    render_exit();
    printf("%f, ", player_body->aabb.pos[1]);

    glfwDestroyWindow(rendering_state.window);
    glfwTerminate();
    return 0;
}

static void handle_input(void) {
    if (keys.escape != KEY_UNPRESSED) app_running = false;

    float32 velx = 0;
    float32 vely = player_body->velocity[1];

    if (keys.left != KEY_UNPRESSED) velx -= 100;
    if (keys.right != KEY_UNPRESSED) velx += 100;
    if (keys.up != KEY_UNPRESSED) vely = 400;
    if (keys.down != KEY_UNPRESSED) vely -= 80;

    player_body->velocity[0] = velx;
    player_body->velocity[1] = vely;
}
