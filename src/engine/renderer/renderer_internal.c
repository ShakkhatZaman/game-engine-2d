#include <glad/glad.h>
#include <stdio.h>
#include <stddef.h>

#include "renderer_internal.h"

void render_quad_init(uint32 *vao, uint32 *vbo, uint32 *ebo) {
    float32 vertices[] = {
         0.5,  0.5, 0, 0, 0,
         0.5, -0.5, 0, 0, 1,
        -0.5, -0.5, 0, 1, 1,
        -0.5,  0.5, 0, 1, 0,
    };

    uint32 indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, ebo);

    glBindVertexArray(*vao);

    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*vertices) * 20, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices) * 6, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(*vertices), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(*vertices), (void *) (3 * sizeof(*vertices)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void render_batch_quads_init(uint32 *vao, uint32 *vbo, uint32 *ebo) {
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    uint32 indices[MAX_INDICES];
    for (uint32 i = 0, v = 0; i < MAX_INDICES && v < MAX_VERTICES; i += 6, v += 4) {
        indices[0 + i] = v + 0;
        indices[1 + i] = v + 1;
        indices[2 + i] = v + 2;
        indices[3 + i] = v + 2;
        indices[4 + i] = v + 3;
        indices[5 + i] = v + 0;
    }

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(B_vertex), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(B_vertex), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(B_vertex), (void *)offsetof(B_vertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(B_vertex), (void *)offsetof(B_vertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(B_vertex), (void *)offsetof(B_vertex, texture_slot));
    glEnableVertexAttribArray(3);

    glGenBuffers(1, ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(*indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void render_line_init(uint32 *vao, uint32 *vbo) {
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float32), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float32), NULL);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void append_batch_quad(vec2 pos, vec2 size, vec4 uv, vec4 color, int32 texture_slot) {
    vec4 default_uv = {0, 0, 1, 1};
    if (uv)
        memcpy(default_uv, uv, 4 * sizeof(*default_uv));
    if (texture_slot == -1)
        texture_slot = 0;

    list_append(batch_vert_list, &(B_vertex){
        .pos = {pos[0], pos[1]},
        .uv = {default_uv[0], default_uv[1]},
        .color = {color[0], color[1], color[2], color[3]},
        .texture_slot = texture_slot
    });
    list_append(batch_vert_list, &(B_vertex){
        .pos = {pos[0] + size[0], pos[1]},
        .uv = {default_uv[2], default_uv[1]},
        .color = {color[0], color[1], color[2], color[3]},
        .texture_slot = texture_slot
    });
    list_append(batch_vert_list, &(B_vertex){
        .pos = {pos[0] + size[0], pos[1] + size[1]},
        .uv = {default_uv[2], default_uv[3]},
        .color = {color[0], color[1], color[2], color[3]},
        .texture_slot = texture_slot
    });
    list_append(batch_vert_list, &(B_vertex){
        .pos = {pos[0], pos[1] + size[1]},
        .uv = {default_uv[0], default_uv[3]},
        .color = {color[0], color[1], color[2], color[3]},
        .texture_slot = texture_slot
    });
}

void render_shaders_init(void) {
    default_shader = shader_create("./res/shaders/default.vert", "./res/shaders/default.frag");
    batch_shader = shader_create("./res/shaders/batch_quad.vert", "./res/shaders/batch_quad.frag");
    mat4x4_ortho(projection, 0, app_width, 0, app_height, -2, 2);
    glUseProgram(default_shader);
    glUniformMatrix4fv(
        glGetUniformLocation(default_shader, "projection"),
        1, GL_FALSE, &projection[0][0]
    );
    glUseProgram(batch_shader);
    glUniformMatrix4fv(
        glGetUniformLocation(batch_shader, "projection"),
        1, GL_FALSE, &projection[0][0]
    );

    for (int i = 0; i < 8; i++) {
        char uniform_name[] = "texture_slot_N";
        sprintf(uniform_name, "texture_slot_%d", i);
        glUniform1i(
            glGetUniformLocation(batch_shader, uniform_name), i
        );
    }
    glUseProgram(0);
    batch_vert_list = list_create(8, sizeof(B_vertex));
}

void render_textures_init(uint32 *texture) {
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    uint8 white[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    glBindTexture(GL_TEXTURE_2D, 0);
}
