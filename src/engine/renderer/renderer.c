#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"
#include "../global.h"
#include "../utils.h"

Renderer_state_internal state;

void render_quad_init(uint32 *vao, uint32 *vbo, uint32 *ebo);
void render_shaders_init(void);

GLFWwindow *_create_window(int32 width, int32 height);

void render_init(void) {
    rendering_state.width = WINDOW_WIDTH;
    rendering_state.height = WINDOW_HEIGHT;
    rendering_state.window = _create_window(rendering_state.width, rendering_state.height);
    if (!rendering_state.window) {
        ERROR_EXIT_PROGRAM("Exiting in render init\n");
    }

    render_quad_init(&state.vao, &state.vbo, &state.ebo);
    render_shaders_init();
}

void render_begin(void) {
    glClearColor(0.08, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void render_end(void) {
    glfwSwapBuffers(rendering_state.window);
    glfwPollEvents();
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
    glUseProgram(state.default_shader);
    glBindVertexArray(state.vao);

    mat4x4 model;
    mat4x4_identity(model);
    mat4x4_translate(model, pos[0], pos[1], 0);
    mat4x4_scale_aniso(model, model, size[0], size[1], 0);

    glUniformMatrix4fv(
        glGetUniformLocation(state.default_shader, "projection"),
        1, GL_FALSE, &state.projection[0][0]
    );
    glUniform4fv(
        glGetUniformLocation(state.default_shader, "color"),
        1, &color[0]
    );
    glUniformMatrix4fv(
        glGetUniformLocation(state.default_shader, "model"),
        1, GL_FALSE, &model[0][0]
    );

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}

void render_shaders_init(void) {
    state.default_shader = shader_create("./shaders/default.vert", "./shaders/default.frag");
    mat4x4_ortho(state.projection, 0, (float32) rendering_state.width, 0, (float32) rendering_state.height, -2, 2);
}

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


GLFWwindow *_create_window(int32 width, int32 height) {
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
