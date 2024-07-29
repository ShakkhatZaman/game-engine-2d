#include "physics.h"
#include "../list.h"
#include "../utils.h"
#include "../global.h"
#include <math.h>

typedef struct physics_internal_state {
    float32 gravity, terminal_velocity;
    List *body_list, *static_body_list;
} Physics_internal_state;

static Physics_internal_state state;

static uint32 iterations = 4;
static float32 tick_rate;

static void stationary_response(Body *body);
static void sweep_response(Body *body, vec2 velocity);
static Collision sweep_static_bodies(AABB *aabb, vec2 velocity);

void physics_init(void) {
    state.body_list = list_create(0, sizeof(Body));
    state.static_body_list = list_create(0, sizeof(Static_body));

    state.gravity = -20;
    state.terminal_velocity = -1000;

    tick_rate = 1.0 / iterations;
}

void physics_update(void) {
    Body *body;
    for (uint64 i = 0; i < state.body_list->len; i++) {
        body = list_get(state.body_list, i);

        body->velocity[1] += state.gravity;
        // limit y velocity to terminal velocity
        if (state.terminal_velocity > body->velocity[1])
            body->velocity[1] = state.terminal_velocity;

        body->velocity[0] += body->acceleration[0];
        body->velocity[1] += body->acceleration[1];

        // scale velocity with delta time to use in calculations
        vec2 scaled_velocity;
        vec2_scale(scaled_velocity, body->velocity, timing.delta * tick_rate);
        for (int j = 0; j < iterations; j++) {
            sweep_response(body, scaled_velocity);
            stationary_response(body);
        }
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

uint64 physics_static_body_create(vec2 pos, vec2 size) {
    Static_body static_body = {
        .aabb = {
            .pos = { pos[0], pos[1] },
            .half_size = { size[0] * 0.5, size[1] * 0.5 }
        },
    };
    if (list_append(state.static_body_list, &static_body) == (uint64)-1) {
        ERROR_EXIT_PROGRAM("Cannot append item to physics static_body_list\n");
    }
    return state.static_body_list->len - 1;
}
Static_body *physics_static_body_get(uint64 index) {
    return (Static_body *)list_get(state.static_body_list, index);
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
        if (magnitude[i] != 0) {
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

        float32 dx = result.pos[0] - aabb.pos[0];
        float32 dy = result.pos[1] - aabb.pos[1];
        float32 px = aabb.half_size[0] - fabsf(dx);
        float32 py = aabb.half_size[1] - fabsf(dy);

        // set normal for the direction of collision
        if (px < py) 
            result.normal[0] = (dx > 0) - (dx < 0);
        else
            result.normal[1] = (dy > 0) - (dy < 0);
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

    // get default minimum distance to move the object
    float32 min_dist_signed = fabsf(min[0]) < fabsf(min[1]) ? min[0] : min[1];
    float32 min_dist = fabsf(min_dist_signed);

    result[0] = min_dist_signed;
    result[1] = 0;

    // set the correct distance to move the object
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

static void stationary_response(Body *body) {
    for (uint32 i = 0; i < state.static_body_list->len; i++) {
        Static_body *static_body = physics_static_body_get(i);
        AABB aabb = minkowsky_diff_aabb(&static_body->aabb, &body->aabb);

        vec2 min, max;
        aabb_min_max(min, max, &aabb);
        if (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0) {
            // move object according to penetration vector
            vec2 penetration_vector;
            minkowsky_diff_pen_vector(penetration_vector, &aabb);
            vec2_add(body->aabb.pos, body->aabb.pos, penetration_vector);
        }
    }
}
static void sweep_response(Body *body, vec2 distance) {
    Collision collision = sweep_static_bodies(&body->aabb, distance);

    if (collision.collided) {
        body->aabb.pos[0] = collision.pos[0];
        body->aabb.pos[1] = collision.pos[1];

        // reset velocity in the direction of collision and move in the other direction
        if (collision.normal[0] != 0) {
            body->aabb.pos[1] += distance[1];
            body->velocity[0] = 0;
        }
        else if (collision.normal[1] != 0) {
            body->aabb.pos[0] += distance[0];
            body->velocity[1] = 0;
        }
    }
    else {
        // no collisions
        vec2_add(body->aabb.pos, body->aabb.pos, distance);
    }
}

static Collision sweep_static_bodies(AABB *aabb, vec2 velocity) {
    Collision result = {.time = 0xBEEF};

    for (uint32 i = 0; i < state.static_body_list->len; i++) {
        Static_body *static_body = physics_static_body_get(i);
        AABB sum_aabb = static_body->aabb;
        // calculate collision aabb
        vec2_add(sum_aabb.half_size, sum_aabb.half_size, aabb->half_size);
        // cast a ray from the object's position to collsion rectangle
        Collision hit = ray_collide_aabb(aabb->pos, velocity, sum_aabb);
        if (!hit.collided)
            continue;

        if (hit.time < result.time)
            result = hit;
        else if (hit.time == result.time) {
            // solve highest velocity axis first
            if (fabsf(velocity[0]) > fabsf(velocity[1]) && hit.normal[0] != 0)
                result = hit;    
            else if (fabsf(velocity[1]) > fabsf(velocity[0]) && hit.normal[1] != 0)
                result = hit;
        }
    }
    return result;
}

void physics_exit(void) {
    list_delete(state.body_list);
    list_delete(state.static_body_list);
}
