#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/global.h"
#include "engine/utils.h"
#include "engine/renderer/renderer.h"
#include "engine/physics/physics.h"
#include "engine/entities/entities.h"
#include "engine/animation.h"

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
    GLFWwindow *window = render_init();
    config_init();
    physics_init();
    entity_init();
    animation_init();
    glfwSetKeyCallback(window, glfw_key_callback);
    
    uint8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
    uint8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
    
    uint64 player_id = entity_create((Body_data){
                                     .pos = {300, 100}, .size = {25, 25},
                                     .velocity = {0, 0}, .collision_layer = COLLISION_LAYER_PLAYER, .collision_mask = player_mask},
                                     player_on_hit_callback, player_on_static_hit_callback);
    float32 width = 640, height = 360;
    uint32 static_body_a_id = physics_static_body_create((Body_data){.pos = {width * 0.5 - 12.5, height - 12.5}, .size = {width - 25, 25}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint32 static_body_b_id = physics_static_body_create((Body_data){.pos = {width - 12.5, height * 0.5 + 12.5}, .size = {25, height - 25}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint32 static_body_c_id = physics_static_body_create((Body_data){.pos = {width * 0.5 + 12.5, 12.5}, .size = {width - 25, 25}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint32 static_body_d_id = physics_static_body_create((Body_data){.pos = {12.5, height * 0.5 - 12.5}, .size = {25, height - 25}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint32 static_body_e_id = physics_static_body_create((Body_data){.pos = {width * 0.5, height * 0.5}, .size = {50, 50}, .collision_layer = COLLISION_LAYER_TERRAIN});
    uint64 enemy_id_a = entity_create((Body_data){
                                     .pos = {300, 100}, .size = {25, 25},
                                     .velocity = {90, 0}, .collision_layer = COLLISION_LAYER_ENEMY, .collision_mask = enemy_mask},
                                     NULL, enemy_on_static_hit_callback);

    Sprite_sheet player_sprite_sheet;
    render_load_sprite_sheet(&player_sprite_sheet, "./res/assets/player.png", 24, 24);

    uint64 player_walk_animation_def_id = animation_def_create(
        &player_sprite_sheet,
        (float32[]){0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1},
        (uint8[]){1, 1, 1, 1, 1, 1, 1},
        (uint8[]){1, 2, 3, 4, 5, 6, 7}, 7
    );
    uint64 player_idle_animation_def_id = animation_def_create(
        &player_sprite_sheet,
        (float32[]){0}, (uint8[]){0}, (uint8[]){0}, 1
    );
    uint64 player_walk_animation_id = animation_create(player_walk_animation_def_id, true);
    uint64 player_idle_animation_id = animation_create(player_idle_animation_def_id, true);

    while(app_running) {
        time_update();

        Entity *player = entity_get(player_id);
        player_body = physics_body_get(player->body_id);
        Static_body *static_body_a = physics_static_body_get(static_body_a_id);
        Static_body *static_body_b = physics_static_body_get(static_body_b_id);
        Static_body *static_body_c = physics_static_body_get(static_body_c_id);
        Static_body *static_body_d = physics_static_body_get(static_body_d_id);
        Static_body *static_body_e = physics_static_body_get(static_body_e_id);
        Entity *enemy = entity_get(enemy_id_a);
        enemy_body = physics_body_get(enemy->body_id);

        if (player_body->velocity[0] != 0)
            player->animation_id = player_walk_animation_id;
        else
            player->animation_id = player_idle_animation_id;

        handle_input();
        physics_update();
        animation_update(timing.delta);
        render_begin();

        render_aabb(&player_body->aabb, player_color);
        render_aabb(&enemy_body->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_a->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_b->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_c->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_d->aabb, (vec4){1, 1, 1, 1});
        render_aabb(&static_body_e->aabb, (vec4){1, 1, 1, 1});

        for (uint64 i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);
            if (!entity->active) continue;
            if (entity->animation_id == -1) continue;
            Body *body = physics_body_get(entity->body_id);
            Animation *anim = animation_get(entity->animation_id);
            Animation_frame *frame = &anim->def->frames[anim->current_frame_index];

            if (body->velocity[0] < 0) anim->is_flipped = true;
            else if (body->velocity[0] > 0) anim->is_flipped = false;

            render_sprite_sheet_frame(anim->def->sheet,
                                      frame->row, frame->col, body->aabb.pos,
                                      (vec2){25, 25}, anim->is_flipped);
        }

        render_sprite_sheet_frame(&player_sprite_sheet, 1, 2, (vec2){100, 200}, (vec2){50, 50}, false);
        render_sprite_sheet_frame(&player_sprite_sheet, 0, 3, (vec2){50, 100}, (vec2){25, 25}, false);

        render_end(window, &width, &height, player_sprite_sheet.texture_id);
        time_update_end();
        player_color[0] = 0;
        player_color[2] = 1;
    }
    render_exit();
    physics_exit();
    entity_exit();
    animation_exit();
    glDeleteTextures(1, &player_sprite_sheet.texture_id);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

static void handle_input(void) {
    if (keys.escape != KEY_UNPRESSED) app_running = false;

    float32 velx = 0;
    float32 vely = player_body->velocity[1];

    if (keys.left != KEY_UNPRESSED) velx -= 200;
    if (keys.right != KEY_UNPRESSED) velx += 200;
    if (keys.up != KEY_UNPRESSED && player_on_ground) {
        player_on_ground = false;
        vely = 1500;
    }
    // if (keys.down != KEY_UNPRESSED) vely -= 80;

    player_body->velocity[0] = velx;
    player_body->velocity[1] = vely;
}
