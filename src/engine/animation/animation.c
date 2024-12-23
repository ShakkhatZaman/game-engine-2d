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
    ASSERT_RETURN(MAX_FRAMES > frame_count, -1, "Animation frame count higher than max value\n");

    Animation_def def = {
        .sheet = sheet, .frame_count = frame_count
    };
    for (uint8 i = 0; i < frame_count; i++) {
        def.frames[i] = (Animation_frame) {
            .col = cols[i], .row = row,
            .duration = duration
        };
    }
    uint64 animation_def_id = list_append(animation_def_list, &def);
    ASSERT_RETURN(animation_def_id != -1, -1, "Unable to add to animation_def_list");
    return animation_def_id;
}

uint64 animation_create(uint64 animation_def_id, bool does_loop) {
    uint64 id = animation_list->len;
    ASSERT_RETURN(animation_def_id != -1, -1, "Illegal Animation definition id to create animation\n");
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
        ASSERT_RETURN(id != -1, -1, "Unable to add item in animation_list\n");
    }
    animation = list_get(animation_list, id);
    *animation = (Animation){
        .def_id = animation_def_id, .does_loop = does_loop,
        .active = true
    };
    return id;
}

Animation *animation_get(uint64 animation_id) {
    Animation * anim = list_get(animation_list, animation_id);
    ASSERT_RETURN(anim, NULL, "Cannot access item in animation_list\n");
    return anim;
}

void animation_update(float32 dt) {
    for (uint64 i = 0; i < animation_list->len; i++) {
        Animation *animation = list_get(animation_list, i);
        Animation_def *def = list_get(animation_def_list, animation->def_id);
        animation->current_frame_duration -= dt;

        if (animation->current_frame_duration <= 0) {
            animation->current_frame_index += 1;
            if (animation->current_frame_index >= def->frame_count) {
                animation->current_frame_index = (animation->does_loop) ? 0 : def->frame_count - 1;
            }
            animation->current_frame_duration = def->frames[animation->current_frame_index].duration;
        }
    }
}

void animation_render(uint64 animation_id, vec2 pos, vec2 size, vec4 color) {
    ASSERT_RETURN(animation_id != -1, (void) 0, "Illegal Animation id to render from\n");
    Animation *anim = animation_get(animation_id);
    if (anim->active) {
        ASSERT_RETURN(anim->def_id != -1, (void) 0, "Illegal Animation definition id to render from\n");
        Animation_def *def = list_get(animation_def_list, anim->def_id);

        Animation_frame *frame = &def->frames[anim->current_frame_index];
        render_sprite_sheet_frame(def->sheet, frame->row, frame->col, pos, size, color, anim->is_flipped);
    }
}

void animation_destroy(uint64 animation_id) {
    ASSERT_RETURN(animation_id != -1, (void) 0, "Illegal Animation id to destroy\n");
    Animation *animation = list_get(animation_list, animation_id);
    animation->active = false;
}

void animation_exit(void) {
    list_delete(animation_list);
    list_delete(animation_def_list);
}
