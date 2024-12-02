#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdbool.h>
#include <linmath.h>
#include "../types.h"

typedef enum collision_layer {
    COLLISION_LAYER_PLAYER = 1,
    COLLISION_LAYER_ENEMY = 1 << 1,
    COLLISION_LAYER_TERRAIN = 1 << 2,
    COLLISION_LAYER_ENEMY_PASSTHROUGH = 1 << 3,
} Collision_layer;

typedef struct body Body;
typedef struct collision Collision;
typedef struct static_body Static_body;

typedef void (*On_hit)(Body *self, Body *other, Collision *hit);
typedef void (*On_static_hit)(Body *self, Static_body *other, Collision *hit);

typedef struct aabb {
    vec2 pos, half_size;
} AABB;

typedef struct body_data {
    vec2 pos, size, velocity;
    uint8 collision_layer, collision_mask;
} Body_data;

struct body {
    AABB aabb;
    vec2 velocity, acceleration;
    uint8 collision_layer, collision_mask;
    On_hit on_hit;
    On_static_hit on_static_hit;
    uint64 entity_id;
    bool active, kinematic;
};

struct static_body {
    AABB aabb;
    uint8 collision_layer;
};

struct collision {
    bool collided;
    float32 time;
    vec2 pos;
    vec2 normal;
    uint64 other_id;
};

void physics_init(void);
void physics_update(void);
void physics_exit(void);

uint64 physics_body_create(Body_data *data, bool kinematic, On_hit on_hit, On_static_hit on_static_hit);
uint64 physics_trigger_create(vec2 position, vec2 size, uint8 collision_layer, uint8 collision_mask, On_hit on_hit);
uint64 physics_body_count(void);
Body *physics_body_get(uint64 index);

uint64 physics_static_body_create(Body_data data);
uint64 physics_static_body_count(void);
Static_body *physics_static_body_get(uint64 index);

bool physics_point_intersect(vec2 point, AABB *aabb);
void aabb_min_max(vec2 min, vec2 max, AABB *aabb);
AABB minkowsky_diff_aabb(AABB *a, AABB *b);
bool physics_aabb_intersect(AABB *a, AABB *b);

void minkowsky_diff_pen_vector(vec2 result, AABB *minkowsky_aabb);
Collision ray_collide_aabb(vec2 position, vec2 magnitude, AABB aabb);

#endif // !PHYSICS_H
