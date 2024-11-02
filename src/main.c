#include <stdbool.h>

#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "engine/global.h"
#include "engine/utils.h"
#include "engine/renderer/renderer.h"
#include "engine/physics/physics.h"
#include "engine/entities/entities.h"
#include "engine/animation/animation.h"
#include "engine/audio/audio.h"

static float32 SMALL_ENEMY_SPEED = 100;
static float32 LARGE_ENEMY_SPEED = 150;
static float32 SMALL_ENEMY_HEALTH = 3;
static float32 LARGE_ENEMY_HEALTH = 7;

static float32 spawn_timer = 0;
static float32 width = 640, height = 360;

static SDL_Event event;
static Mix_Music *MUSIC_STAGE_1;
static Mix_Chunk *JUMP_SOUND;

static void handle_input(void);
static bool app_running = true;
static Entity *player = NULL;
static Body *player_body = NULL;

static vec4 player_color = {0, 1, 1, 1};
static bool player_on_ground = false;

static uint64 player_walk_animation_id, player_idle_animation_id;
static Sprite_sheet player_sprites, map_sprites, enemy_small_sprites, enemy_large_sprites, props_sprites;

static uint64 player_walk_animation_def_id;
static uint64 player_idle_animation_def_id;
static uint64 small_enemy_animation_def_id;
static uint64 large_enemy_animation_def_id;

void player_on_hit_callback(Body *self, Body *other, Collision *collision);
void player_on_static_hit_callback(Body *self, Static_body *other, Collision *collision);
void small_enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision);
void large_enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision);
void fire_on_hit(Body *self, Body *other, Collision *collision);

void spawn_enemy(bool is_large, bool is_raged, bool is_flipped);

int main(void) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        ERROR_RETURN(1, "Unable to initialize SDL. SDL error: %s\n", SDL_GetError());
    }

    time_init(60);
    SDL_Window *window = render_init();
    config_init();
    physics_init();
    entity_init();
    animation_init();
    audio_init();

    audio_music_load(&MUSIC_STAGE_1, "./res/sounds/breezys_mega_quest_2_stage_1.mp3");
    audio_sound_load(&JUMP_SOUND, "./res/sounds/jump.wav");
    audio_play_music(MUSIC_STAGE_1, -1);

    uint8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
    uint8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN | COLLISION_LAYER_ENEMY_PASSTHROUGH;
    uint8 fire_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER;

    uint64 player_id = entity_create(&(Body_data){
                                     .pos = {300, 150}, .size = {25, 25},
                                     .velocity = {0, 0}, .collision_layer = COLLISION_LAYER_PLAYER, .collision_mask = player_mask},
                                     (vec4){0, 0}, false, player_on_hit_callback, player_on_static_hit_callback);
    physics_static_body_create((Body_data){.pos = {width * 0.5, height - 16}, .size = {width, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width * 0.25, 19}, .size = {width * 0.5 - 64, 38}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width * 0.75, 19}, .size = {width * 0.5 - 64, 38}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {16, height * 0.5 - 2 * 32}, .size = {32, height - 128}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width - 16, height * 0.5 - 2 * 32}, .size = {32, height - 128}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {60, height - 32 * 3 - 16}, .size = {200, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width - 60, height - 32 * 3 - 16}, .size = {200, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width * 0.5, height - 32 * 3 - 16}, .size = {192, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width * 0.5, 32 * 3 + 24}, .size = {448, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {16, height - 64}, .size = {32, 64}, .collision_layer = COLLISION_LAYER_ENEMY_PASSTHROUGH});
    physics_static_body_create((Body_data){.pos = {width - 16, height - 64}, .size = {32, 64}, .collision_layer = COLLISION_LAYER_ENEMY_PASSTHROUGH});
    uint64 fire_id = entity_create(&(Body_data){
                                     .pos = {width * 0.5, -4}, .size = {64, 8},
                                     .velocity = {0, 0}, .collision_layer = 0, .collision_mask = fire_mask},
                                     (vec4){0, 0}, true, fire_on_hit, NULL);

    render_load_sprite_sheet(&player_sprites, "./res/textures/player.png", 24, 24);
    render_load_sprite_sheet(&map_sprites, "./res/textures/map.png", 640, 360);
    render_load_sprite_sheet(&enemy_large_sprites, "./res/textures/enemy_large.png", 40, 40);
    render_load_sprite_sheet(&enemy_small_sprites, "./res/textures/enemy_small.png", 24, 24);
    render_load_sprite_sheet(&props_sprites, "./res/textures/props_16x16.png", 16, 16);

    player_walk_animation_def_id = animation_def_create(&player_sprites, 0.1, 0, (uint8[]){1, 2, 3, 4, 5, 6, 7}, 7);
    player_idle_animation_def_id = animation_def_create(&player_sprites, 0, 0, (uint8[]){0}, 1);
    small_enemy_animation_def_id = animation_def_create(&enemy_small_sprites, 0.12, 1, (uint8[]){0, 1, 2, 3, 4, 5, 6, 7}, 8);
    large_enemy_animation_def_id = animation_def_create(&enemy_large_sprites, 0.12, 1, (uint8[]){0, 1, 2, 3, 4, 5, 6, 7}, 8);
    player_walk_animation_id = animation_create(player_walk_animation_def_id, true);
    player_idle_animation_id = animation_create(player_idle_animation_def_id, true);
    float32 spawn_time = 0;

    while(app_running) {
        time_update();

        player = entity_get(player_id);
        player_body = physics_body_get(player->body_id);

        handle_input();
        physics_update();
        animation_update(timing.delta);
        render_begin();

        render_sprite_sheet_frame(&map_sprites, 0, 0, (vec4){width / 2, height / 2}, (vec4){640, 360}, (vec4){1, 1, 1, 0.5}, false);

        for (uint64 i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);
            if (!entity->active) continue;
            if (entity->animation_id == -1) continue;
            Body *body = physics_body_get(entity->body_id);
            Animation *anim = animation_get(entity->animation_id);

            if (body->velocity[0] < 0) anim->is_flipped = true;
            else if (body->velocity[0] > 0) anim->is_flipped = false;

            //sprite center position
            vec2 pos;
            vec2_add(pos, body->aabb.pos, entity->sprite_offset);
            animation_render(entity->animation_id, pos, (vec2){-1, -1}, (vec4){1, 1, 1, 1});
        }

        for (uint64 i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);
            // if (!entity->active) continue;
            Body *body = physics_body_get(entity->body_id);
            if (body->active) {
                render_aabb(&body->aabb, (vec4){0.25, 0.25, 1, 1});
            } else {
                render_aabb(&body->aabb, (vec4){1, 0, 0, 1});
            }
        }

        for (int i = 0; i < physics_static_body_count(); i++) {
            Static_body *body = physics_static_body_get(i);
            render_aabb(&body->aabb, (vec4){1, 1, 1, 1});
        }

        render_end(window, &width, &height);
        time_update_end();
        player_color[0] = 0;
        player_color[2] = 1;

        {
            spawn_timer -= 2 * timing.delta;
            if (spawn_timer <= 0) {
                spawn_timer = (float32)((rand() % 200) + 200) / 100.0;
                spawn_enemy(false, false, true);
                spawn_enemy(true, true, false);
            }
        }
        char FPS[10];
        int printed = snprintf(FPS, 10, "FPS: %d", timing.frame_rate);
        if (printed > 9)
            SDL_SetWindowTitle(window, "frame_rate_truncated");
        else
            SDL_SetWindowTitle(window, FPS);
    }
    render_exit();
    physics_exit();
    entity_exit();
    animation_exit();
    glDeleteTextures(1, &player_sprites.texture_id);
    glDeleteTextures(1, &map_sprites.texture_id);
    glDeleteTextures(1, &enemy_large_sprites.texture_id);
    glDeleteTextures(1, &enemy_small_sprites.texture_id);
    glDeleteTextures(1, &props_sprites.texture_id);
    Mix_FreeChunk(JUMP_SOUND);
    Mix_FreeMusic(MUSIC_STAGE_1);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

static void handle_input(void) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) app_running = false;
    }
    input_update();

    if (keys[KEY_ESCAPE] != KEY_UNPRESSED) app_running = false;

    float32 velx = 0;
    float32 vely = player_body->velocity[1];

    Animation *player_walk_animation = animation_get(player_walk_animation_id);
    Animation *player_idle_animation = animation_get(player_idle_animation_id);
    player->animation_id = player_idle_animation_id;

    if (keys[KEY_LEFT] != KEY_UNPRESSED) {
        velx -= 350;
        player_walk_animation->is_flipped = true;
        player_idle_animation->is_flipped = true;
        player->animation_id = player_walk_animation_id;
    }
    if (keys[KEY_RIGHT] != KEY_UNPRESSED) {
        velx += 350;
        player_walk_animation->is_flipped = false;
        player_idle_animation->is_flipped = false;
        player->animation_id = player_walk_animation_id;
    }
    if (keys[KEY_UP] != KEY_UNPRESSED && player_on_ground) {
        vely = 1200;
        audio_play_sound(JUMP_SOUND);
    }
    // if (keys.down != KEY_UNPRESSED) vely -= 80;

    player_body->velocity[0] = velx;
    player_body->velocity[1] = vely;
    player_on_ground = false;
}

void player_on_hit_callback(Body *self, Body *other, Collision *collision) {
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        player_color[0] = 1;
        player_color[2] = 0;
    }
}

void player_on_static_hit_callback(Body *self, Static_body *other, Collision *collision) {
    if (collision->normal[1] > 0) {
        player_on_ground = true;
        self->velocity[1] = 0;
    }
}

void small_enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision) {
    if (collision->normal[0] > 0) {
        self->velocity[0] = SMALL_ENEMY_SPEED;
    }
    if (collision->normal[0] < 0) {
        self->velocity[0] = -SMALL_ENEMY_SPEED;
    }
}

void large_enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision) {
    if (collision->normal[0] > 0) {
        self->velocity[0] = LARGE_ENEMY_SPEED;
    }
    if (collision->normal[0] < 0) {
        self->velocity[0] = -LARGE_ENEMY_SPEED;
    }
}

void fire_on_hit(Body *self, Body *other, Collision *collision) {
    if (other->collision_layer == COLLISION_LAYER_PLAYER) {
        other->aabb.pos[0] = 300;
        other->aabb.pos[1] = 150;
    }
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        for (uint64 i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);
            if (entity->body_id == collision->other_id) {
                entity_destroy(i);
                animation_destroy(entity->animation_id);
                break;
            }
        }
    }
}

void spawn_enemy(bool is_large, bool is_raged, bool is_flipped) {
    float32 speed = is_large ? LARGE_ENEMY_SPEED : SMALL_ENEMY_SPEED;
    // float32 speed = 0;
    vec2 size = {16, 16};
    vec2 sprite_offset = {0, 5};
    if (is_large) {
        size[0] = 34; size[1] = 22;
        sprite_offset[0] = 0; sprite_offset[1] = 10;
    }
    Body_data enemy_body_data = {
        .pos = {is_flipped ? width + 30 : -30, 280},
        .size = { size[0], size[1] }, .velocity = {is_flipped ? -speed : speed, 0},
        .collision_layer = COLLISION_LAYER_ENEMY, .collision_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN
    };
    uint64 entity_id = entity_create(
        &enemy_body_data, sprite_offset, false, NULL,
        is_large ? large_enemy_on_static_hit_callback : small_enemy_on_static_hit_callback);
    Entity *enemy_entity = entity_get(entity_id);
    uint64 animation_id = animation_create(
        is_large ? large_enemy_animation_def_id : small_enemy_animation_def_id, true
    );
    enemy_entity->animation_id = animation_id;
}
