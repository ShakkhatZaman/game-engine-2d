#ifndef ENTITIES_H
#define ENTITIES_H

#include <stdbool.h>
#include <linmath.h>
#include "../types.h"
#include "../physics/physics.h"

typedef struct entity {
    uint64 body_id, animation_id;
    bool active;
    vec2 sprite_offset;
} Entity;

void entity_init(void);
uint64 entity_create(Body_data *data, vec2 sprite_offset, bool kinematic, On_hit on_hit, On_static_hit on_static_hit);
Entity *entity_get(uint64 id);
void entity_destroy(uint64 entity_id);
bool entity_damage(uint64 entity_id, uint8 damage);
uint64 entity_count(void);
void entity_exit(void);

#endif /* ifndef ENTITIES_H */
