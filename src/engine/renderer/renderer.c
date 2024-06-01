#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "../global.h"
#include "../utils.h"

typedef struct renderer_state_internal {
    uint32 vao_quad, vbo_quad, ebo_quad;
    uint32 vao_line, vbo_line;
    uint32 default_shader, texture_color;
    mat4x4 projection;
    uint8 texture_slot;
} Renderer_state_internal;

Renderer_state_internal state;

static void render_quad_init(uint32 *vao, uint32 *vbo, uint32 *ebo);
static void render_line_init(uint32 *vao, uint32 *vbo);
static void render_shaders_init(uint32 *shader);
static void render_textures_init(uint32 *texture);

static GLFWwindow *_create_window(int32 width, int32 height);

void render_init(void) {
    rendering_state.width = WINDOW_WIDTH;
    rendering_state.height = WINDOW_HEIGHT;
    rendering_state.window = _create_window(rendering_state.width, rendering_state.height);
    if (!rendering_state.window) {
        ERROR_EXIT_PROGRAM("Exiting in render init\n");
    }

    render_quad_init(&state.vao_quad, &state.vbo_quad, &state.ebo_quad);
    render_line_init(&state.vao_line, &state.vbo_line);
    render_shaders_init(&state.default_shader);
    state.texture_slot = 0;
    render_textures_init(&state.texture_color);
}

void render_begin(void) {
    glClearColor(0.08, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void render_end(void) {
    glfwSwapBuffers(rendering_state.window);
    glfwPollEvents();
}

void render_exit(void) {
    glDeleteBuffers(1, &state.vbo_quad);
    glDeleteBuffers(1, &state.vbo_line);
    glDeleteBuffers(1, &state.ebo_quad);
    glDeleteVertexArrays(1, &state.vao_quad);
    glDeleteVertexArrays(1, &state.vao_line);
    glDeleteProgram(state.default_shader);
    glDeleteTextures(1, &state.texture_color);
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
    glUseProgram(state.default_shader);
    glBindVertexArray(state.vao_quad);

    mat4x4 model;
    mat4x4_translate(model, pos[0], pos[1], 0);
    mat4x4_scale_aniso(model, model, size[0], size[1], 0);

    glUniformMatrix4fv(
        glGetUniformLocation(state.default_shader, "model"),
        1, GL_FALSE, &model[0][0]
    );
    glUniform4fv(
        glGetUniformLocation(state.default_shader, "color"),
        1, &color[0]
    );
    glUniform1i(
        glGetUniformLocation(state.default_shader, "texture_id"), 0
    );

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glActiveTexture(GL_TEXTURE0 + state.texture_slot);
    glBindTexture(GL_TEXTURE_2D, state.texture_color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void render_line_segment(vec2 start, vec2 end, vec4 color) {
    glUseProgram(state.default_shader);
    glLineWidth(3);

    float32 x = end[0] - start[0];
    float32 y = end[1] - start[1];
    float32 line[6] = {0, 0, 0, x, y, 0};

    mat4x4 model;
    mat4x4_translate(model, start[0], start[1], 0);

    glUniformMatrix4fv(
        glGetUniformLocation(state.default_shader, "model"),
        1, GL_FALSE, &model[0][0]
    );
    glUniform4fv(
        glGetUniformLocation(state.default_shader, "color"),
        1, &color[0]
    );
    glUniform1i(
        glGetUniformLocation(state.default_shader, "texture_id"), 0
    );


    glBindVertexArray(state.vao_line);
    glActiveTexture(GL_TEXTURE0 + state.texture_slot);
    glBindTexture(GL_TEXTURE_2D, state.texture_color);

    glBindBuffer(GL_ARRAY_BUFFER, state.vbo_line);
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

static void render_shaders_init(uint32 *shader) {
    *shader = shader_create("./shaders/default.vert", "./shaders/default.frag");
    mat4x4_ortho(state.projection, 0, (float32) rendering_state.width, 0, (float32) rendering_state.height, -2, 2);
    glUseProgram(state.default_shader);
    glUniformMatrix4fv(
        glGetUniformLocation(state.default_shader, "projection"),
        1, GL_FALSE, &state.projection[0][0]
    );
    glUseProgram(0);
}

static void render_textures_init(uint32 *texture) {
    glGenTextures(1, texture);
    glActiveTexture(GL_TEXTURE0 + state.texture_slot);
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
