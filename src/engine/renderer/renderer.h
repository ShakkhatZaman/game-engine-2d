#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <linmath.h>

#include "../physics/physics.h"
#include "../types.h"

#define MAX_QUADS 1000
#define MAX_VERTICES 4000
#define MAX_INDICES 6000

typedef struct batch_vertex {
    vec2 pos, uv;
    vec4 color;
} B_vertex;

typedef struct sprite_sheet {
    float32 width, height;
    float32 cell_width, cell_height;
    uint32 texture_id;
} Sprite_sheet;

SDL_Window *render_init(void);
void render_begin(void);
void render_end(SDL_Window *window, float32 *m_width, float32 *m_height, uint32 batch_texture_id);
void render_exit(void);

uint32 shader_create(const char *vert_shader_path, const char *frag_shader_path);
void shader_init(void);

void render_quad(vec2 pos, vec2 size, vec4 color);
void render_quad_line(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void render_aabb(AABB *aabb, vec4 color);

void render_load_sprite_sheet(Sprite_sheet *sheet, const char *path, float32 cell_width, float32 cell_height);
void render_sprite_sheet_frame(Sprite_sheet *sheet, float32 row, float32 col, vec2 pos, vec2 size, bool is_flipped);

#endif // !RENDERER_H
