#include "animation.h"
#include "../list.h"
#include "../utils.h"

static List *animation_def_list;
static List *animation_list;

void animation_init(void) {
    animation_def_list = list_create(0, sizeof(Animation_def));
    animation_list = list_create(0, sizeof(Animation));
}

uint64 animation_def_create(Sprite_sheet *sheet, float32 duration, uint8 row, uint8 *cols, uint8 frame_count) {
    if (frame_count > MAX_FRAMES) {
        ERROR_EXIT_PROGRAM("Animation frame count higher than max value\n");
    }

    Animation_def def = {
        .sheet = sheet, .frame_count = frame_count
    };
    for (uint8 i = 0; i < frame_count; i++) {
        def.frames[i] = (Animation_frame) {
            .col = cols[i], .row = row,
            .duration = duration
        };
    }
    return list_append(animation_def_list, &def);
}

uint64 animation_create(uint64 animation_def_id, bool does_loop) {
    uint64 id = animation_list->len;
    if (animation_def_id < 0) {
        ERROR_EXIT_PROGRAM("Animation definition with id: %zu not found\n", animation_def_id);
    }
    Animation *animation;
    // Find an inactive animation first
    for (uint64 i = 0; i < animation_list->len; i++) {
        animation = list_get(animation_list, i);
        if (!animation->active) {
            id = i;
            break;
        };
    }

    if (id >= animation_list->len) {
        id = list_append(animation_list, &(Animation){0});
    }
    animation = list_get(animation_list, id);
    *animation = (Animation){
        .def_id = animation_def_id, .does_loop = does_loop,
        .active = true
    };
    return id;
}

Animation *animation_get(uint64 animation_id) {
    return list_get(animation_list, animation_id);
}

void animation_update(float32 dt) {
    for (uint64 i = 0; i < animation_list->len; i++) {
        Animation *animation = list_get(animation_list, i);
        Animation_def *def = list_get(animation_def_list, animation->def_id);
        animation->current_frame_duration -= dt;

        if (animation->current_frame_duration <= 0) {
            animation->current_frame_index += 1;
 
            if(animation->current_frame_index >= def->frame_count) {
                animation->current_frame_index = (animation->does_loop) ? 0 : def->frame_count - 1;
            }

            animation->current_frame_duration = def->frames[animation->current_frame_index].duration;
        }
    }
}

void animation_render(uint64 animation_id, vec2 pos, vec2 size, vec4 color) {
    if (animation_id == -1) {
        ERROR_EXIT("Animation not found");
        return;
    }
    Animation *anim = animation_get(animation_id);
    Animation_def *def = list_get(animation_def_list, anim->def_id);
    Animation_frame *frame = &def->frames[anim->current_frame_index];
    render_sprite_sheet_frame(def->sheet, frame->row, frame->col, pos, size, color, anim->is_flipped);
}

void animation_destroy(uint64 animation_id) {
    Animation *animation = list_get(animation_list, animation_id);
    animation->active = false;
}

void animation_exit(void) {
    list_delete(animation_list);
    list_delete(animation_def_list);
}
