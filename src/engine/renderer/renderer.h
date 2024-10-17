#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <linmath.h>

#include "../physics/physics.h"
#include "../types.h"

typedef struct sprite_sheet {
    float32 width, height;
    float32 cell_width, cell_height;
    uint32 texture_id;
} Sprite_sheet;

SDL_Window *render_init(void);
void render_begin(void);
void render_end(SDL_Window *window, float32 *m_width, float32 *m_height);
void render_exit(void);

void render_quad(vec2 pos, vec2 size, vec4 color);
void render_quad_line(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void render_aabb(AABB *aabb, vec4 color);
void render_batch(uint32 count, uint32 texture_id[]);

void render_load_sprite_sheet(Sprite_sheet *sheet, const char *path, float32 cell_width, float32 cell_height);
void render_sprite_sheet_frame(Sprite_sheet *sheet, float32 row, float32 col, vec2 pos, vec2 size, bool is_flipped);

#endif // !RENDERER_H
