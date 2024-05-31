#include "physics.h"
#include "../list.h"
#include "../utils.h"
#include "../global.h"


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
