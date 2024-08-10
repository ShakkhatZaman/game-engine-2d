#include <stddef.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "../utils.h"
#include "../list.h"

static uint32 vao_quad, vbo_quad, ebo_quad;
static uint32 vao_batch, vbo_batch, ebo_batch;
static uint32 vao_line, vbo_line;
static uint32 default_shader, batch_shader;
static uint32 texture_color;
static float32 ortho_left, ortho_bottom;
static mat4x4 projection, model_global;
static uint8 texture_slot;
static float32 window_width = 1280, window_height = 720;
static float32 render_width = 640, render_height = 360;
static List *batch_vert_list = NULL;

static void render_quad_init(uint32 *vao, uint32 *vbo, uint32 *ebo);
static void render_batch_quads_init(uint32 *vao, uint32 *vbo, uint32 *ebo);
static void render_line_init(uint32 *vao, uint32 *vbo);
static void render_shaders_init(void);
static void render_textures_init(uint32 *texture);
static void render_batch(uint32 count, uint32 texture_id);

static GLFWwindow *_create_window(int32 width, int32 height);

GLFWwindow *render_init(void) {
    GLFWwindow *window = _create_window(window_width, window_height);
    if (!window) {
        ERROR_EXIT_PROGRAM("Exiting in render init\n");
    }

    render_quad_init(&vao_quad, &vbo_quad, &ebo_quad);
    render_line_init(&vao_line, &vbo_line);
    render_shaders_init();
    render_batch_quads_init(&vao_batch, &vbo_batch, &ebo_batch);
    texture_slot = 0;
    render_textures_init(&texture_color);
    mat4x4_identity(model_global);
    glViewport(0, 0, window_width, window_height);
    ortho_left = 0;
    ortho_bottom = 0;
    return window;
}

void render_begin(void) {
    glClearColor(0.08, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    batch_vert_list->len = 0;
}

void render_end(GLFWwindow *window, float32 *m_width, float32 *m_height) {
    render_batch(batch_vert_list->len, texture_color);
    glfwSwapBuffers(window);
    glfwPollEvents();
    int32 new_width, new_height;
    glfwGetWindowSize(window, &new_width, &new_height);
    if (new_width != window_width || new_height != window_height) {
        float32 scale_x = (float32) new_width / window_width, scale_y = (float32) new_height / window_height;
        float32 scale = (scale_x < scale_y) ? scale_x : scale_y;
        float32 scaled_width = window_width * scale, scaled_height = window_height * scale;
        glViewport(0, 0, (int32) scaled_width, (int32) scaled_height);
        glfwSetWindowSize(window, (int32) scaled_width, (int32) scaled_height);
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
    glUniform1i(
        glGetUniformLocation(default_shader, "texture_id"), texture_slot
    );

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
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
    glUniform1i(
        glGetUniformLocation(default_shader, "texture_id"), texture_slot
    );

    glBindVertexArray(vao_line);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
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

void append_batch_quad(vec2 pos, vec2 size, vec4 uv, vec4 color) {
    vec4 default_uv = {0, 0, 1, 1};
    if (uv)
        memcpy(default_uv, uv, 4 * sizeof(*default_uv));

    list_append(batch_vert_list, &(B_vertex){
        .pos = {pos[0], pos[1]},
        .uv = {default_uv[0], default_uv[1]},
        .color = {color[0], color[1], color[2], color[3]}
    });
    list_append(batch_vert_list, &(B_vertex){
        .pos = {pos[0] + size[0], pos[1]},
        .uv = {default_uv[2], default_uv[1]},
        .color = {color[0], color[1], color[2], color[3]}
    });
    list_append(batch_vert_list, &(B_vertex){
        .pos = {pos[0] + size[0], pos[1] + size[1]},
        .uv = {default_uv[2], default_uv[3]},
        .color = {color[0], color[1], color[2], color[3]}
    });
    list_append(batch_vert_list, &(B_vertex){
        .pos = {pos[0], pos[1] + size[1]},
        .uv = {default_uv[0], default_uv[3]},
        .color = {color[0], color[1], color[2], color[3]}
    });
}

static void render_batch(uint32 count, uint32 texture_id) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_batch);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(B_vertex), batch_vert_list->items);

    glUseProgram(batch_shader);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glBindVertexArray(vao_batch);

    glDrawElements(GL_TRIANGLES, (count / 2) * 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

static void render_shaders_init(void) {
    default_shader = shader_create("./shaders/default.vert", "./shaders/default.frag");
    batch_shader = shader_create("./shaders/batch_quad.vert", "./shaders/batch_quad.frag");
    mat4x4_ortho(projection, 0, render_width, 0, render_height, -2, 2);
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
    glUseProgram(0);
    batch_vert_list = list_create(8, sizeof(B_vertex));
}

static void render_textures_init(uint32 *texture) {
    glGenTextures(1, texture);
    glActiveTexture(GL_TEXTURE0 + texture_slot);
    glBindTexture(GL_TEXTURE_2D, *texture);

    uint8 white[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void render_quad_init(uint32 *vao, uint32 *vbo, uint32 *ebo) {
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

static void render_batch_quads_init(uint32 *vao, uint32 *vbo, uint32 *ebo) {
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

    glGenBuffers(1, ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(*indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void render_line_init(uint32 *vao, uint32 *vbo) {
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

static GLFWwindow *_create_window(int32 width, int32 height) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, "Hello world", NULL, NULL);
    if (!window) {
        glfwTerminate();
        ERROR_RETURN(NULL, "Unable to create window\n");
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        glfwTerminate();
        glfwDestroyWindow(window);
        ERROR_RETURN(NULL, "Unable to load OpenGL\n");
    }

    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Version: %s\n", glGetString(GL_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));

    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return window;
}
