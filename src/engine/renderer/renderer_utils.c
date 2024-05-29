#include <glad/glad.h>

#include "../io/io.h"
#include "../types.h"
#include "renderer.h"
#include "../utils.h"

static uint32 _compile_shader(const void *shader_src, GLenum shader_type);

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
        glGetShaderInfoLog(shader, message_len, NULL, message);
        glDeleteProgram(shader);
        ERROR_RETURN(0, "Unable to link shader\n Error: %s\n", message);
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &status);
    if (!status) {
        int message_len = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &message_len);
        char message[message_len];
        glGetShaderInfoLog(shader, message_len, NULL, message);
        glDeleteProgram(shader);
        ERROR_RETURN(0, "Unable to link shader\n Error: %s\n", message);
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    free(vert_shader_src.data);
    free(frag_shader_src.data);
    return shader;
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

