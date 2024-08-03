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
#include "engine/entities/entities.h"

static void handle_input(void);
static bool app_running = true;
static Body *player_body = NULL, *enemy_body = NULL;

static vec4 player_color = {0, 1, 1, 1};
static bool player_on_ground = false;

void player_on_hit_callback(Body *self, Body *other, Collision *collision) {
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        player_color[0] = 1;
        player_color[2] = 0;
    }
}

void player_on_static_hit_callback(Body *self, Static_body *other, Collision *collision) {
    if (collision->normal[1] > 0) {
        player_on_ground = true;
    }
}

void enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision) {
    if (collision->normal[0] > 0) {
        self->velocity[0] = 100;
    }
    if (collision->normal[0] < 0) {
        self->velocity[0] = -100;
    }
}

int main(void) {
    if(!glfwInit()) {ERROR_RETURN(1, "Unable to initialize GLFW\n");}

    time_init(60);
    render_init();
    config_init();
    physics_init();
    entity_init();
    glfwSetKeyCallback(rendering_state.window, glfw_key_callback);
    
    uint8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
    uint8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
    
    uint64 player_id = entity_create((Body_data){
                                     .pos = {300, 100}, .size = {50, 50},
                                     .velocity = {0, 0}, .collision_layer = COLLISION_LAYER_PLAYER, .collision_mask = player_mask},
                                     player_on_hit_callback, player_on_static_hit_callback);
    float32 width = rendering_state.width;
    float32 height = rendering_state.height;
    uint32 static_body_a_id = physics_static_body_create((Body_data){.pos = {width * 0.5 - 10, height - 10}, .size = {width - 20, 20}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint32 static_body_b_id = physics_static_body_create((Body_data){.pos = {width - 10, height * 0.5 + 10}, .size = {20, height - 20}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint32 static_body_c_id = physics_static_body_create((Body_data){.pos = {width * 0.5 + 10, 10}, .size = {width - 20, 20}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint32 static_body_d_id = physics_static_body_create((Body_data){.pos = {10, height * 0.5 - 10}, .size = {20, height - 20}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint32 static_body_e_id = physics_static_body_create((Body_data){.pos = {width * 0.5, height * 0.5}, .size = {50, 50}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint64 enemy_id_a = entity_create((Body_data){
                                     .pos = {300, 100}, .size = {50, 50},
                                     .velocity = {90, 0}, .collision_layer = COLLISION_LAYER_ENEMY, .collision_mask = enemy_mask},
                                     NULL, enemy_on_static_hit_callback);

    while(app_running) {
        time_update();

        player_body = physics_body_get(player_id);
        Static_body *static_body_a = physics_static_body_get(static_body_a_id);
        Static_body *static_body_b = physics_static_body_get(static_body_b_id);
        Static_body *static_body_c = physics_static_body_get(static_body_c_id);
        Static_body *static_body_d = physics_static_body_get(static_body_d_id);
        Static_body *static_body_e = physics_static_body_get(static_body_e_id);
        enemy_body = physics_body_get(enemy_id_a);

        handle_input();
        physics_update();
        render_begin();

        render_aabb(&player_body->aabb, player_color);
        render_aabb(&enemy_body->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_a->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_b->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_c->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_d->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_e->aabb, (vec4){1, 1, 1, 1});

        render_end();
        time_update_end();
        player_color[0] = 0;
        player_color[2] = 1;
    }
    render_exit();
    physics_exit();
    entity_exit();

    glfwDestroyWindow(rendering_state.window);
    glfwTerminate();
    return 0;
}

static void handle_input(void) {
    if (keys.escape != KEY_UNPRESSED) app_running = false;

    float32 velx = 0;
    float32 vely = player_body->velocity[1];

    if (keys.left != KEY_UNPRESSED) velx -= 400;
    if (keys.right != KEY_UNPRESSED) velx += 400;
    if (keys.up != KEY_UNPRESSED && player_on_ground) {
        player_on_ground = false;
        vely = 900;
    }
    if (keys.down != KEY_UNPRESSED) vely -= 80;

    player_body->velocity[0] = velx;
    player_body->velocity[1] = vely;
}
