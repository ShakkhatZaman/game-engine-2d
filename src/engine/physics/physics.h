#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdbool.h>
#include <linmath.h>
#include "../types.h"

typedef struct aabb {
    vec2 pos, half_size;
} AABB;

typedef struct body {
    AABB aabb;
    vec2 velocity, acceleration;
} Body;

void physics_init(void);
void physics_update(void);
uint64 physics_body_create(vec2 pos, vec2 size);
Body *physics_body_get(uint64 index);
bool physics_point_intersect(vec2 point, AABB *aabb);
AABB minkowsky_diff_aabb(AABB *a, AABB *b);
bool physics_aabb_intersect(AABB *a, AABB *b);
void minkowsky_diff_pen_vector(vec2 result, AABB *minkowsky_aabb);

#endif // !PHYSICS_H
