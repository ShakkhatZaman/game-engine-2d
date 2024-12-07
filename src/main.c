#include <math.h>
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
#include "engine/weapons.h"

static float32 PLAYER_SPEED = 350, PLAYER_JUMP_VELOCITY = 1200;
static float32 SMALL_ENEMY_SPEED = 100, LARGE_ENEMY_SPEED = 150;
static float32 SMALL_ENEMY_HEALTH = 3, LARGE_ENEMY_HEALTH = 7;

static int32 total_enemy_count = 3, current_enemy_spawn_counter = 0;
static float32 spawn_timer = 0;
static float32 width = 640, height = 360;

static uint8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
static uint8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN | COLLISION_LAYER_ENEMY_PASSTHROUGH;
static uint8 fire_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER;
static uint8 projectile_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;

static SDL_Event event;
static Mix_Music *MUSIC_STAGE_1;
static Mix_Chunk *JUMP_SOUND;

static void handle_input(void);
static bool app_running = true;
static bool player_died = false;
static uint64 player_id = 0, player_spawn_timer = 0;
static Entity *player = NULL;
static Body *player_body = NULL;

#ifdef _DEBUG_
static vec4 environment_color = {1, 1, 1, 0.5};
#else
static vec4 environment_color = {1, 1, 1, 1};
#endif

static float32 projectile_timer = 0;
static float32 player_direction = 1;
static vec4 player_color = {0, 1, 1, 1};
static bool player_on_ground = false;
static Weapon_type current_weapon = WEAPON_PISTOL;

// animation creation related variables
static uint64 player_walk_animation_id, player_idle_animation_id;
static Sprite_sheet player_sprites, map_sprites, enemy_small_sprites, enemy_large_sprites, props_sprites, fire_sprites;

static uint64 player_walk_animation_def_id, player_idle_animation_def_id;
static uint64 small_enemy_animation_def_id, large_enemy_animation_def_id;
static uint64 small_raged_enemy_animation_def_id, large_raged_enemy_animation_def_id;
static uint64 fire_animation_def_id, fire_animation_id;
static uint64 projectile_animation_id;

void player_on_hit_callback(Body *self, Body *other, Collision *collision);
void player_on_static_hit_callback(Body *self, Static_body *other, Collision *collision);
void small_enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision);
void large_enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision);
void fire_on_hit(Body *self, Body *other, Collision *collision);
void projectile_on_static_hit_callback(Body *self, Static_body *other, Collision *collision);
void shoot_gun(void);

uint64 spawn_player(void);
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

    // Environmental static bodies creation
    physics_static_body_create((Body_data){.pos = {width * 0.5, height - 16}, .size = {width, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width * 0.25, 20}, .size = {width * 0.5 - 64, 38}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width * 0.75, 20}, .size = {width * 0.5 - 64, 38}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {16, height * 0.5 - 2 * 32}, .size = {32, height - 128}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width - 16, height * 0.5 - 2 * 32}, .size = {32, height - 128}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {60, height - 32 * 3 - 16}, .size = {200, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width - 60, height - 32 * 3 - 16}, .size = {200, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width * 0.5, height - 32 * 3 - 16}, .size = {192, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {width * 0.5, 32 * 3 + 24}, .size = {448, 32}, .collision_layer = COLLISION_LAYER_TERRAIN});
    physics_static_body_create((Body_data){.pos = {16, height - 64}, .size = {32, 64}, .collision_layer = COLLISION_LAYER_ENEMY_PASSTHROUGH});
    physics_static_body_create((Body_data){.pos = {width - 16, height - 64}, .size = {32, 64}, .collision_layer = COLLISION_LAYER_ENEMY_PASSTHROUGH});

    // Regular entities creation
    player_id = spawn_player();
    uint64 fire_trigger_id = physics_trigger_create((vec2){width * 0.5, -4}, (vec2){64, 8}, 0, fire_mask, fire_on_hit);

    //timers
    player_spawn_timer = timer_create(2000, false);

    // Sprite sheets loading
    render_load_sprite_sheet(&player_sprites, "./res/textures/player.png", 24, 24);
    render_load_sprite_sheet(&map_sprites, "./res/textures/map.png", 640, 360);
    render_load_sprite_sheet(&enemy_large_sprites, "./res/textures/enemy_large.png", 40, 40);
    render_load_sprite_sheet(&enemy_small_sprites, "./res/textures/enemy_small.png", 24, 24);
    render_load_sprite_sheet(&props_sprites, "./res/textures/props_16x16.png", 16, 16);
    render_load_sprite_sheet(&fire_sprites, "./res/textures/fire.png", 32, 64);

    // Entity animation creation
    player_walk_animation_def_id = animation_def_create(&player_sprites, 0.1, 0, (uint8[]){1, 2, 3, 4, 5, 6, 7}, 7);
    player_idle_animation_def_id = animation_def_create(&player_sprites, 0, 0, (uint8[]){0}, 1);
    small_enemy_animation_def_id = animation_def_create(&enemy_small_sprites, 0.12, 1, (uint8[]){0, 1, 2, 3, 4, 5, 6, 7}, 8);
    large_enemy_animation_def_id = animation_def_create(&enemy_large_sprites, 0.12, 1, (uint8[]){0, 1, 2, 3, 4, 5, 6, 7}, 8);
    small_raged_enemy_animation_def_id = animation_def_create(&enemy_small_sprites, 0.12, 0, (uint8[]){0, 1, 2, 3, 4, 5, 6, 7}, 8);
    large_raged_enemy_animation_def_id = animation_def_create(&enemy_large_sprites, 0.12, 0, (uint8[]){0, 1, 2, 3, 4, 5, 6, 7}, 8);
    player_walk_animation_id = animation_create(player_walk_animation_def_id, true);
    player_idle_animation_id = animation_create(player_idle_animation_def_id, true);
    fire_animation_def_id = animation_def_create(&fire_sprites, 0.1, 0, (uint8[]){0, 1, 2, 3, 4, 5, 6}, 7);
    fire_animation_id = animation_create(fire_animation_def_id, true);
    projectile_animation_id = animation_create(player_idle_animation_def_id, true);

    uint64 fire_id = entity_create(&(Body_data){.pos = {width * 0.5, 10}, .size = {32, 64}, .kinematic = true,}, ENTITY_FIRE, (vec2){0, 0}, NULL, NULL, NULL);
    Entity *fire = entity_get(fire_id);
    fire->animation_id = fire_animation_id;

    fire_id = entity_create(&(Body_data){.pos = {width * 0.5 + 16, 0}, .size = {32, 64}, .kinematic = true,}, ENTITY_FIRE, (vec2){0, 0}, NULL, NULL, NULL);
    fire = entity_get(fire_id);
    fire->animation_id = fire_animation_id;

    fire_id = entity_create(&(Body_data){.pos = {width * 0.5 - 16, 0}, .size = {32, 64}, .kinematic = true,}, ENTITY_FIRE, (vec2){0, 0}, NULL, NULL, NULL);
    fire = entity_get(fire_id);
    fire->animation_id = fire_animation_id;

    float32 spawn_time = 0;
    current_enemy_spawn_counter = total_enemy_count;

    while(app_running) {
        time_update();

        if (player_died && timer_check_complete(player_spawn_timer)) {
            player_id = spawn_player();
            player_died = false;
        }
        player = (!player_died) ? entity_get(player_id) : NULL;
        player_body = (!player_died) ? physics_body_get(player->body_id) : NULL;

        handle_input();
        physics_update();
        animation_update(timing.delta);
        render_begin();

        // Rendering sprites and evironment
        // environment
        render_sprite_sheet_frame(&map_sprites, 0, 0, (vec4){width / 2, height / 2}, (vec4){640, 360}, (vec4){1, 1, 1, 0.5}, false);

        // all sprites
        for (uint64 i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);
            if (!entity->active) continue;
            if (entity->animation_id == -1) continue;
            Body *body = physics_body_get(entity->body_id);
            Animation *anim = animation_get(entity->animation_id);

            if (body->velocity[0] < -1) anim->is_flipped = true;
            else if (body->velocity[0] > 1) anim->is_flipped = false;

            //sprite center position
            vec2 pos;
            vec2_add(pos, body->aabb.pos, entity->sprite_offset);
            animation_render(entity->animation_id, pos, (vec2){-1, -1}, (vec4){1, 1, 1, 1});
        }

#ifdef _DEBUG_
        for (uint64 i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);
            // if (!entity->active) continue;
            Body *body = physics_body_get(entity->body_id);
            if (body->active) {
                render_aabb(&body->aabb, (vec4){0.25, 0.25, 1, 1});
            }
            else {
                render_aabb(&body->aabb, (vec4){1, 0, 0, 1});
            }
        }

        for (int i = 0; i < physics_static_body_count(); i++) {
            Static_body *body = physics_static_body_get(i);
            render_aabb(&body->aabb, (vec4){1, 1, 1, 1});
        }
#endif

        render_end(window, &width, &height);
        time_update_end();
        player_color[0] = 0;
        player_color[2] = 1;

        spawn_timer -= timing.delta;
        if (spawn_timer <= 0) {
            spawn_timer = (float32)((rand() % 200) + 200) / 100.0;
            // for (int i = 0; i < current_enemy_spawn_counter; i++) {
            if (current_enemy_spawn_counter > 0) {
                spawn_enemy(rand() % 2, false, rand() % 2);
                current_enemy_spawn_counter -= 1;
            }
            // }
            if (current_enemy_spawn_counter < 1)
                current_enemy_spawn_counter = total_enemy_count;
        }
        projectile_timer -= (projectile_timer <= 0) ? 0 : timing.delta;

        // show fps
        char FPS[10];
        int printed = snprintf(FPS, 10, "FPS: %d", timing.frame_rate);
        if (printed > 9)
            SDL_SetWindowTitle(window, "frame_rate_truncated");
        else
            SDL_SetWindowTitle(window, FPS);
    }
    // Exiting program
    time_exit();
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

    if (!player_died) {
        float32 velx = 0;
        float32 vely = player_body->velocity[1];

        Animation *player_walk_animation = animation_get(player_walk_animation_id);
        Animation *player_idle_animation = animation_get(player_idle_animation_id);
        player->animation_id = player_idle_animation_id;

        if (keys[KEY_LEFT] != KEY_UNPRESSED) {
            velx -= PLAYER_SPEED;
            player_walk_animation->is_flipped = true;
            player_idle_animation->is_flipped = true;
            player->animation_id = player_walk_animation_id;
            player_direction = -1;
        }
        if (keys[KEY_RIGHT] != KEY_UNPRESSED) {
            velx += PLAYER_SPEED;
            player_walk_animation->is_flipped = false;
            player_idle_animation->is_flipped = false;
            player->animation_id = player_walk_animation_id;
            player_direction = 1;
        }
        if (keys[KEY_UP] != KEY_UNPRESSED && player_on_ground) {
            vely = PLAYER_JUMP_VELOCITY;
            audio_play_sound(JUMP_SOUND);
        }
        if (keys[KEY_SHOOT] != KEY_UNPRESSED) {
            if (projectile_timer <= 0) {
                shoot_gun();
                projectile_timer = 0.1 / weapons[current_weapon].fire_rate;
            }
        }
        // if (keys.down != KEY_UNPRESSED) vely -= 80;

        player_body->velocity[0] = velx;
        player_body->velocity[1] = vely;
        player_on_ground = false;
    }
}

// Entity callbacks
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
    if (collision->normal[1] < 0) {
        self->velocity[1] = 0;
    }
}

void small_enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision) {
    if (collision->normal[0] > 0) {
        self->velocity[0] = fabsf(self->velocity[0]);
    }
    if (collision->normal[0] < 0) {
        self->velocity[0] = -fabsf(self->velocity[0]);
    }
    if (collision->normal[1] > 0) {
        self->velocity[1] = 0;
    }
}

void large_enemy_on_static_hit_callback(Body *self, Static_body *other, Collision *collision) {
    if (collision->normal[0] > 0) {
        self->velocity[0] = fabsf(self->velocity[0]);
    }
    if (collision->normal[0] < 0) {
        self->velocity[0] = -fabsf(self->velocity[0]);
    }
    if (collision->normal[1] > 0) {
        self->velocity[1] = 0;
    }
}

void fire_on_hit(Body *self, Body *other, Collision *collision) {
    if (other->collision_layer == COLLISION_LAYER_PLAYER) {
        ASSERT_RETURN(other->entity_id != -1, (void) 0, "Illegal player entity_id  in body struct\n");
        player_died = true;
        entity_destroy(player_id);
        timer_restart(player_spawn_timer);
    }
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        ASSERT_RETURN(other->entity_id != -1, (void) 0, "Illegal enemy entity_id  in body struct\n");
        Entity *entity = entity_get(other->entity_id);
        Entity_type entity_type = entity->type;
        ASSERT_RETURN(entity->animation_id != -1, (void) 0, "Illegal Animation id in entity struct\n");
        animation_destroy(entity->animation_id);
        entity_destroy(other->entity_id);
        bool is_large = (entity_type == ENTITY_ENEMY_LARGE) ? true : false;
        spawn_enemy(is_large, true, rand() % 2);
        current_enemy_spawn_counter -= 1;
    }
}

void projectile_on_static_hit_callback(Body *self, Static_body *other, Collision *collision) {
    if (collision->normal[0] != 0) {
        uint64 projectile_id = self->entity_id;
        entity_destroy(projectile_id);
    }
}
void projectile_on_hit_callback(Body *self, Body *other, Collision *collision) {
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        uint64 projectile_id = self->entity_id;
        entity_destroy(projectile_id);
        Entity *enemy = entity_get(other->entity_id);
        animation_destroy(enemy->animation_id);
        entity_destroy(other->entity_id);
    }
}

// Spawns one of the two types of enemies
void spawn_enemy(bool is_large, bool is_raged, bool is_flipped) {
    Entity *enemy_entity;
    uint64 animation_id;

    if (is_large) {
        vec2 size = {34, 22};
        vec2 sprite_offset = {0, 8};
        float32 speed = LARGE_ENEMY_SPEED;
        speed *= (is_raged ? 1.25 : 1);
        Body_data enemy_body_data = {
            .pos = {is_flipped ? width + 30 : -30, 280},
            .size = { size[0], size[1] }, .velocity = {is_flipped ? -speed : speed, 0},
            .collision_layer = COLLISION_LAYER_ENEMY, .collision_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN
        };
        uint64 entity_id = entity_create(
            &enemy_body_data, ENTITY_ENEMY_LARGE, sprite_offset, NULL,
            large_enemy_on_static_hit_callback, NULL);
        ASSERT_EXIT(entity_id != -1, "Cannot spawn enemy entity");

        enemy_entity = entity_get(entity_id);

        animation_id = animation_create(
            is_raged ? large_raged_enemy_animation_def_id : large_enemy_animation_def_id, true
        );
        ASSERT_EXIT(animation_id != -1, "Cannot create player entity");
    }
    else {
        vec2 size = {16, 16};
        vec2 sprite_offset = {0, 3};
        float32 speed = SMALL_ENEMY_SPEED;
        speed *= (is_raged ? 1.25 : 1);
        Body_data enemy_body_data = {
            .pos = {is_flipped ? width + 30 : -30, 280},
            .size = { size[0], size[1] }, .velocity = {is_flipped ? -speed : speed, 0},
            .collision_layer = COLLISION_LAYER_ENEMY, .collision_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN
        };
        uint64 entity_id = entity_create(
            &enemy_body_data, ENTITY_ENEMY_SMALL, sprite_offset, NULL,
            small_enemy_on_static_hit_callback, NULL);
        ASSERT_EXIT(entity_id != -1, "Cannot spawn enemy entity");

        enemy_entity = entity_get(entity_id);

        animation_id = animation_create(
            is_raged ? small_raged_enemy_animation_def_id : small_enemy_animation_def_id, true
        );
        ASSERT_EXIT(animation_id != -1, "Cannot create player entity");
    }
    enemy_entity->animation_id = animation_id;
}

uint64 spawn_player(void) {
    Body_data body_data = {
        .pos = {300, 150}, .size = {25, 25},
        .velocity = {0, 0}, .collision_layer = COLLISION_LAYER_PLAYER, .collision_mask = player_mask};

    uint64 spawned_player_id = entity_create(&body_data, ENTITY_PLAYER, (vec4){0, 0},
                                     player_on_hit_callback, player_on_static_hit_callback, NULL);
    ASSERT_EXIT(spawned_player_id != -1, "Cannot create player entity");
    return spawned_player_id;
}

void shoot_gun(void) {
    float32 velocity = weapons[current_weapon].projectile_speed * player_direction;
    uint64 projectile_id = entity_create(&(Body_data){
        .pos = {player_body->aabb.pos[0], player_body->aabb.pos[1]}, .velocity = {velocity, 0},
        .size = {25, 25}, .kinematic = true, .collision_mask = projectile_mask, .collision_layer = COLLISION_LAYER_PROJECTILE,
    }, ENTITY_PROJECTILE, (vec2){0, 0}, projectile_on_hit_callback, projectile_on_static_hit_callback, NULL);

    Entity *projectile = entity_get(projectile_id);
    projectile->animation_id = projectile_animation_id;
}
