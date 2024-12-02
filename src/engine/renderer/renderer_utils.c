#include <glad/glad.h>

#include "../io/io.h"
#include "../types.h"
#include "renderer_internal.h"
#include "../utils.h"

static uint32 _compile_shader(const void *shader_src, GLenum shader_type);

SDL_Window *create_window(int32 width, int32 height) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_Window *window = SDL_CreateWindow("Hello world", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          width, height, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Quit();
        ERROR_RETURN(NULL, "Unable to create window. SDL error: %s\n", SDL_GetError());
    }

    SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        SDL_Quit();
        SDL_DestroyWindow(window);
        ERROR_RETURN(NULL, "Unable to load OpenGL\n");
    }

#ifdef _DEBUG_
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Version: %s\n", glGetString(GL_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
#endif

    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return window;
}

uint32 shader_create(const char *vert_shader_path, const char *frag_shader_path) {
    int32 status;
    File vert_shader_src = read_file(vert_shader_path);
    File frag_shader_src = read_file(frag_shader_path);
    if (!frag_shader_src.is_valid || !vert_shader_src.is_valid) {
        ERROR_RETURN(0, "Unable to read shader file\n");
    }
    
    uint32 shader = glCreateProgram();
    uint32 vert_shader = _compile_shader(vert_shader_src.data, GL_VERTEX_SHADER);
    uint32 frag_shader = _compile_shader(frag_shader_src.data, GL_FRAGMENT_SHADER);

    glAttachShader(shader, vert_shader);
    glAttachShader(shader, frag_shader);

    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &status);
    if (!status) {
        int message_len = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &message_len);
        char message[message_len];
        glGetProgramInfoLog(shader, message_len, NULL, message);
        glDeleteProgram(shader);
        ERROR_RETURN(0, "Unable to link shader\n Error: %s\n", message);
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &status);
    if (!status) {
        int message_len = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &message_len);
        char message[message_len];
        glGetProgramInfoLog(shader, message_len, NULL, message);
        glDeleteProgram(shader);
        ERROR_RETURN(0, "Unable to link shader\n Error: %s\n", message);
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    free(vert_shader_src.data);
    free(frag_shader_src.data);
    return shader;
}

// takes in texture id
// returns the texture slot number where the texture exists or has been inserted inside the texture_ids list
int32 insert_texture_id(uint32 texture_ids[], uint32 texture_id) {
    for (int i = 1; i < 8; i++) {
        if (texture_ids[i] == texture_id) {
            return i;
        }
        else if (texture_ids[i] == 0) {
            texture_ids[i] = texture_id;
            return i;
        }
    }
    ERROR_RETURN(-1, "Cannot find or insert texture in texture_ids array\n");
}

static uint32 _compile_shader(const void *shader_src, GLenum shader_type) {
    int32 status;
    uint32 shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const char * const *) &shader_src, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        int message_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &message_len);
        char message[message_len];
        glGetShaderInfoLog(shader, message_len, NULL, message);
        glDeleteShader(shader);
        ERROR_RETURN(0, "Unable to compile shader\n Error: %s\n", message);
    }
    return shader;
}

