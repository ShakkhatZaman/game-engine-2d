#ifndef ENTITIES_H
#define ENTITIES_H

#include <stdbool.h>
#include <linmath.h>
#include "../types.h"
#include "../physics/physics.h"

typedef struct entity {
    uint64 body_id, animation_id;
    bool active;
} Entity; 

void entity_init(void);
uint64 entity_create(Body_data *data, bool kinematic, On_hit on_hit, On_static_hit on_static_hit);
Entity *entity_get(uint64 id);
uint64 entity_count(void);
void entity_exit(void);

#endif /* ifndef ENTITIES_H */
