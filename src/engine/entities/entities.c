#include "../list.h"
#include "entities.h"
#include "../utils.h"

static List *entity_list;

void entity_init(void) {
    entity_list = list_create(0, sizeof(Entity));
}

uint64 entity_create(Body_data *data, Entity_type type, vec2 sprite_offset, bool kinematic, On_hit on_hit, On_static_hit on_static_hit) {
    uint64 id = entity_list->len;
    for (uint64 i = 0; i < entity_list->len; i++) {
        Entity *entity = list_get(entity_list, i);
        if (!entity->active) {
            id = i;
            break;
        }
    }

    if (id == entity_list->len) {
        if (list_append(entity_list, &(Entity){0}) == -1) {
            ERROR_EXIT_PROGRAM("Unable to add to entity list\n");
        }
    }
    Entity *entity = list_get(entity_list, id);
    *entity = (Entity){
        .body_id = physics_body_create(data, kinematic, on_hit, on_static_hit),
        .animation_id = -1,
        .active = true, .sprite_offset = {sprite_offset[0], sprite_offset[1]},
        .type = type
    };
    return id;
}

Entity *entity_get(uint64 id) {
    Entity *entity = list_get(entity_list, id);
    if (!entity) {
        ERROR_RETURN(NULL, "error in entity_list\n");
    }
    return entity;
}

uint64 entity_count(void) {
    return entity_list->len;
}

void entity_destroy(uint64 entity_id) {
    Entity *entity = list_get(entity_list, entity_id);
    Body *body = physics_body_get(entity->body_id);
    entity->active = false;
    body->active = false;
}

void entity_exit(void) {
    list_delete(entity_list);
}
