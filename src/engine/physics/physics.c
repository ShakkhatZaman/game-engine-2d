#include "physics.h"
#include "../list.h"
#include "../utils.h"
#include "../global.h"
#include <math.h>


typedef struct physics_internal_state {
    List *body_list;
} Physics_internal_state;

static Physics_internal_state state;

void physics_init(void) {
    state.body_list = list_create(0, sizeof(Body));
}

void physics_update(void) {
    Body *body;
    for (uint64 i = 0; i < state.body_list->len; i++) {
        body = list_get(state.body_list, i);
        body->velocity[0] += body->acceleration[0] * timing.delta;
        body->velocity[1] += body->acceleration[1] * timing.delta;
        body->aabb.pos[0] += body->velocity[0] * timing.delta;
        body->aabb.pos[1] += body->velocity[1] * timing.delta;
    }
}

uint64 physics_body_create(vec2 pos, vec2 size) {
    Body body = {
        .aabb = {
            .pos = { pos[0], pos[1] },
            .half_size = { size[0] * 0.5, size[1] * 0.5 }
        },
        .velocity = { 0, 0 }, .acceleration = { 0, 0 }
    };
    if (list_append(state.body_list, &body) == (uint64)-1) {
        ERROR_EXIT_PROGRAM("Cannot append item to physics body_list\n");
    }
    return state.body_list->len - 1;
}
Body *physics_body_get(uint64 index) {
    return (Body *)list_get(state.body_list, index);
}

bool physics_point_intersect(vec2 point, AABB *aabb) {
    vec2 min, max;
    aabb_min_max(min, max, aabb);
    return point[0] >= min[0] && point[0] <= max[0] &&
           point[1] >= min[1] && point[1] <= max[1];
}

bool physics_aabb_intersect(AABB *a, AABB *b) {
    vec2 min, max;
    AABB diff = minkowsky_diff_aabb(a, b);
    aabb_min_max(min, max, &diff);
    return (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0);
}

Collision ray_collide_aabb(vec2 pos, vec2 magnitude, AABB aabb) {
    Collision result = {0};
    vec2 min, max;
    float32 last_entry = -INFINITY, first_exit = INFINITY;

    aabb_min_max(min, max, &aabb);
    for (uint8 i = 0; i < 2; i++) {
        if (magnitude[0] != 0) {
            float32 t1 = (min[i] - pos[i]) / magnitude[i];
            float32 t2 = (max[i] - pos[i]) / magnitude[i];

            last_entry = fmaxf(last_entry, fminf(t1, t2));
            first_exit = fminf(first_exit, fmaxf(t1, t2));
        }
        else if (pos[i] <= min[i] || pos[i] >= max[i]) {
            return result;
        }
    }
    if (first_exit > last_entry && first_exit > 0 && last_entry < 1) {
        result.pos[0] = pos[0] + magnitude[0] * last_entry;
        result.pos[1] = pos[1] + magnitude[1] * last_entry;
        result.collided = true;
        result.time = last_entry;
    }
    return result;

}

AABB minkowsky_diff_aabb(AABB *a, AABB *b) {
    AABB diff;
    vec2_sub(diff.pos, a->pos, b->pos);
    vec2_add(diff.half_size, a->half_size, b->half_size);
    return diff;
}

void minkowsky_diff_pen_vector(vec2 result, AABB *minkowsky_aabb) {
    vec2 min, max;
    aabb_min_max(min, max, minkowsky_aabb);

    float32 min_dist = fabsf(min[0]);
    result[0] = min[0];
    result[1] = 0;

    if (fabsf(max[0]) < min_dist) {
        min_dist = fabsf(max[0]);
        result[0] = max[0];
    }
    if (fabsf(min[0]) < min_dist) {
        min_dist = fabsf(min[0]);
        result[0] = 0;
        result[1] = min[1];
    }
    if (fabsf(max[1]) < min_dist) {
        result[0] = 0;
        result[1] = max[1];
    }
}

void aabb_min_max(vec2 min, vec2 max, AABB *aabb) {
    vec2_sub(min, aabb->pos, aabb->half_size);
    vec2_add(max, aabb->pos, aabb->half_size);
}
