#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H

#include <SDL2/SDL.h>
#include <linmath.h>
#include "../types.h"
#include "../list.h"

#define MAX_QUADS 1000
#define MAX_VERTICES 4000
#define MAX_INDICES 6000

typedef struct batch_vertex {
    vec2 pos, uv;
    vec4 color;
    int32 texture_slot;
} B_vertex;

extern uint32 vao_quad, vbo_quad, ebo_quad;
extern uint32 vao_batch, vbo_batch, ebo_batch;
extern uint32 vao_line, vbo_line;
extern uint32 default_shader, batch_shader;
extern uint32 texture_color;
extern mat4x4 projection, model_global;
extern float32 window_width, window_height;
extern float32 app_width, app_height;
extern List *batch_vert_list;

SDL_Window *create_window(int32 width, int32 height);
uint32 shader_create(const char *vert_shader_path, const char *frag_shader_path);

void render_quad_init(uint32 *vao, uint32 *vbo, uint32 *ebo);
void render_batch_quads_init(uint32 *vao, uint32 *vbo, uint32 *ebo);
void render_line_init(uint32 *vao, uint32 *vbo);
void render_shaders_init(void);
void render_textures_init(uint32 *texture);

int32 insert_texture_id(uint32 texture_ids[], uint32 texture_id);
void append_batch_quad(vec2 pos, vec2 size, vec4 uv, vec4 color, int32 texture_slot);

#endif // !RENDER_INTERNAL_H
