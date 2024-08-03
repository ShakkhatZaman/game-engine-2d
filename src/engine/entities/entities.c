#include "../list.h"
#include "entities.h"

static List *entity_list;

void entity_init(void) {
    entity_list = list_create(0, sizeof(Entity));
}

uint64 entity_create(Body_data data, On_hit on_hit, On_static_hit on_static_hit) {
    Entity entity = {
        .id = physics_body_create(&data, on_hit, on_static_hit),
        .active = true
    };
    return list_append(entity_list, &entity);
}

Entity *entity_get(uint64 id) {
    return list_get(entity_list, id);
}

uint64 entity_count(void) {
    return entity_list->len;
}

void entity_exit(void) {
    list_delete(entity_list);
}
