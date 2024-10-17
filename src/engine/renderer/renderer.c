#include <stddef.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "renderer.h"
#include "renderer_internal.h"
#include "../utils.h"

uint32 vao_quad, vbo_quad, ebo_quad;
uint32 vao_batch, vbo_batch, ebo_batch;
uint32 vao_line, vbo_line;
uint32 default_shader, batch_shader;
uint32 texture_color;
mat4x4 projection, model_global;
float32 window_width = 1280, window_height = 720;
float32 render_width = 640, render_height = 360;
List *batch_vert_list = NULL;

static uint32 batch_texture_ids[8] = {0};

SDL_Window *render_init(void) {
    SDL_Window *window = create_window(window_width, window_height);
    if (!window) {
        ERROR_EXIT_PROGRAM("Exiting in render init\n");
    }

    stbi_set_flip_vertically_on_load(true);
    render_quad_init(&vao_quad, &vbo_quad, &ebo_quad);
    render_line_init(&vao_line, &vbo_line);
    render_shaders_init();
    render_batch_quads_init(&vao_batch, &vbo_batch, &ebo_batch);
    render_textures_init(&texture_color);
    batch_texture_ids[0] = texture_color;
    mat4x4_identity(model_global);
    glViewport(0, 0, window_width, window_height);
    return window;
}

void render_begin(void) {
    glClearColor(0.08, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    batch_vert_list->len = 0;
}

void render_end(SDL_Window *window, float32 *m_width, float32 *m_height) {
    render_batch(batch_vert_list->len, batch_texture_ids);
    SDL_GL_SwapWindow(window);
    int32 new_width, new_height;
    SDL_GetWindowSize(window, &new_width, &new_height);
    if (new_width != window_width || new_height != window_height) {
        float32 scale_x = (float32) new_width / window_width, scale_y = (float32) new_height / window_height;
        float32 scale = (scale_x < scale_y) ? scale_x : scale_y;
        float32 scaled_width = window_width * scale, scaled_height = window_height * scale;
        glViewport(0, 0, (int32) scaled_width, (int32) scaled_height);
        SDL_SetWindowSize(window, (int32) scaled_width, (int32) scaled_height);
    }
}

void render_exit(void) {
    list_delete(batch_vert_list);
    glDeleteBuffers(1, &vbo_quad);
    glDeleteBuffers(1, &vbo_batch);
    glDeleteBuffers(1, &vbo_line);
    glDeleteBuffers(1, &ebo_quad);
    glDeleteBuffers(1, &ebo_batch);
    glDeleteVertexArrays(1, &vao_quad);
    glDeleteVertexArrays(1, &vao_line);
    glDeleteVertexArrays(1, &vao_batch);
    glDeleteProgram(default_shader);
    glDeleteProgram(batch_shader);
    glDeleteTextures(1, &texture_color);
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
    glUseProgram(default_shader);
    glBindVertexArray(vao_quad);

    mat4x4 model;
    mat4x4_dup(model, model_global);
    mat4x4_translate_in_place(model, pos[0], pos[1], 0);
    mat4x4_scale_aniso(model, model, size[0], size[1], 0);

    glUniformMatrix4fv(
        glGetUniformLocation(default_shader, "model"),
        1, GL_FALSE, &model[0][0]
    );
    glUniform4fv(
        glGetUniformLocation(default_shader, "color"),
        1, &color[0]
    );

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindTexture(GL_TEXTURE_2D, texture_color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void render_line_segment(vec2 start, vec2 end, vec4 color) {
    glUseProgram(default_shader);
    glLineWidth(3);

    float32 x = end[0] - start[0];
    float32 y = end[1] - start[1];
    float32 line[6] = {0, 0, 0, x, y, 0};

    mat4x4 model;
    mat4x4_dup(model, model_global);
    mat4x4_translate_in_place(model, start[0], start[1], 0);

    glUniformMatrix4fv(
        glGetUniformLocation(default_shader, "model"),
        1, GL_FALSE, &model[0][0]
    );
    glUniform4fv(
        glGetUniformLocation(default_shader, "color"),
        1, &color[0]
    );

    glBindVertexArray(vao_line);
    glBindTexture(GL_TEXTURE_2D, texture_color);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(*line), line);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void render_quad_line(vec2 pos, vec2 size, vec4 color) {
    vec2 points[] = {
        {pos[0] - size[0] * 0.5, pos[1] - size[1] * 0.5},
        {pos[0] + size[0] * 0.5, pos[1] - size[1] * 0.5},
        {pos[0] + size[0] * 0.5, pos[1] + size[1] * 0.5},
        {pos[0] - size[0] * 0.5, pos[1] + size[1] * 0.5},
    };
    render_line_segment(points[0], points[1], color);
    render_line_segment(points[1], points[2], color);
    render_line_segment(points[2], points[3], color);
    render_line_segment(points[3], points[0], color);
}

void render_aabb(AABB *aabb, vec4 color) {
    vec2 size;
    vec2_scale(size, aabb->half_size, 2);
    render_quad_line(aabb->pos, size, color);
}

void render_batch(uint32 count, uint32 texture_ids[]) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_batch);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(B_vertex), batch_vert_list->items);

    glUseProgram(batch_shader);
    glBindVertexArray(vao_batch);

    for (int i = 0; i < 8; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, batch_texture_ids[i]);
    }

    glDrawElements(GL_TRIANGLES, (count / 2) * 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    for (int i = 1; i < 8; i++) batch_texture_ids[i] = 0;
}

void render_sprite_sheet_frame(Sprite_sheet *sheet, float32 row, float32 col, vec2 pos, vec2 size, bool is_flipped) {
    vec4 uv;
    float32 norm_cell_width = (1.0 / sheet->width) * sheet->cell_width;
    float32 norm_cell_height = (1.0 / sheet->height) * sheet->cell_height;
    float32 x = col * norm_cell_width, y = row * norm_cell_height;
    uv[0] = x + ((is_flipped) ? norm_cell_width : 0);
    uv[1] = y;
    uv[2] = x + ((is_flipped) ? 0 : norm_cell_width);
    uv[3] = y + norm_cell_height;

    vec2 bottom_left = {pos[0] - size[0] * 0.5, pos[1] - size[1] * 0.5};
    int32 texture_slot = insert_texture_id(batch_texture_ids, sheet->texture_id);
    append_batch_quad(bottom_left, size, uv, (vec4){1, 1, 1, 1}, texture_slot);
}

void render_load_sprite_sheet(Sprite_sheet *sheet, const char *path, float32 cell_width, float32 cell_height) {
    glGenTextures(1, &sheet->texture_id);

    glBindTexture(GL_TEXTURE_2D, sheet->texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int32 width, height, channel_count;
    uint8 *image_data = stbi_load(path, &width, &height, &channel_count, 0);
    if (!image_data) {
        ERROR_EXIT_PROGRAM("Failed to load image : %s.\n", path);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    sheet->width = (float32) width;
    sheet->height = (float32) height;
    sheet->cell_width = cell_width;
    sheet->cell_height = cell_height;
}
