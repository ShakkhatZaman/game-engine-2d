#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "../global.h"
#include "../utils.h"

Renderer_state_internal state;

static void render_quad_init(uint32 *vao, uint32 *vbo, uint32 *ebo);
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

    render_quad_init(&state.vao, &state.vbo, &state.ebo);
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
    glDeleteBuffers(1, &state.vbo);
    glDeleteBuffers(1, &state.ebo);
    glDeleteVertexArrays(1, &state.vao);
    glDeleteProgram(state.default_shader);
    glDeleteTextures(1, &state.texture_color);
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
    glUseProgram(state.default_shader);
    glBindVertexArray(state.vao);

    mat4x4 model;
    mat4x4_identity(model);
    mat4x4_translate(model, pos[0], pos[1], 0);
    mat4x4_scale_aniso(model, model, size[0], size[1], 0);

    glUniform4fv(
        glGetUniformLocation(state.default_shader, "color"),
        1, &color[0]
    );
    glUniformMatrix4fv(
        glGetUniformLocation(state.default_shader, "model"),
        1, GL_FALSE, &model[0][0]
    );
    glUniform1i(
        glGetUniformLocation(state.default_shader, "texture_id"),
        0
    );

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glActiveTexture(GL_TEXTURE0 + state.texture_slot);
    glBindTexture(GL_TEXTURE_2D, state.texture_color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
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
    
    return window;
}
