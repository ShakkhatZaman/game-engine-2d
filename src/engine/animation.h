#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdbool.h>

#include "renderer/renderer.h"
#include "types.h"

#define MAX_FRAMES 16

typedef struct animation_frame {
    float32 duration;
    uint8 row, col;
} Animation_frame;

typedef struct animation_def {
    Sprite_sheet *sheet;
    Animation_frame frames[MAX_FRAMES];
    uint8 frame_count;
} Animation_def;

typedef struct animation {
    Animation_def *def;
    float32 current_frame_duration;
    uint8 current_frame_index;
    bool does_loop, is_active, is_flipped;
} Animation;

void animation_init(void);
uint64 animation_def_create(Sprite_sheet *sheet, float32 *durations, uint8 *rows, uint8 *cols, uint8 frame_count);
uint64 animation_create(uint64 animation_def_id, bool does_loop);
void animation_destroy(uint64 animation_id);
Animation *animation_get(uint64 animation_id);
void animation_update(float32 dt);
void animation_exit(void);

#endif // !ANIMATION_H
