#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>

#include "linmath.h"

#include "../physics/physics.h"
#include "../types.h"

typedef struct renderer_state {
    GLFWwindow *window;
    int32 width;
    int32 height;
} Renderer_state;

void render_init(void);
void render_begin(void);
void render_end(void);
void render_exit(void);

uint32 shader_create(const char *vert_shader_path, const char *frag_shader_path);
void shader_init(void);

void render_quad(vec2 pos, vec2 size, vec4 color);
void render_quad_line(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void render_aabb(AABB *aabb, vec4 color);

#endif // !RENDERER_H
