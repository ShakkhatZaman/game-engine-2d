#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>
#include "../types.h"
#include "linmath.h"

typedef struct renderer_state {
    GLFWwindow *window;
    int32 width;
    int32 height;
} Renderer_state;

typedef struct renderer_state_internal {
    uint32 vao, vbo, ebo;
} Renderer_state_internal;

void render_init(void);
void render_begin(void);
void render_end(void);
void render_exit(void);

void render_quad(vec2 pos, vec2 size, vec4 color);

#endif // !RENDERER_H
