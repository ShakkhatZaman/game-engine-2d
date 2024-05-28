#include <glad/glad.h>

#include "../io/io.h"
#include "../types.h"
#include "renderer.h"
#include "../utils.h"

uint32 _compile_shader(const void *shader_src, GLenum shader_type);

uint32 shader_create(const char *vert_shader_path, const char *frag_shader_path) {
    int32 status;
    File vert_shader_src = read_file(vert_shader_path);
    File frag_shader_src = read_file(frag_shader_path);
    
    uint32 vert_shader = _compile_shader(vert_shader_src.data, GL_VERTEX_SHADER);
    uint32 frag_shader = _compile_shader(frag_shader_src.data, GL_FRAGMENT_SHADER);

    uint32 shader = glCreateProgram();
    glAttachShader(shader, vert_shader);
    glAttachShader(shader, frag_shader);
    glLinkProgram(shader);
    glGetShaderiv(shader, GL_LINK_STATUS, &status);
    if (!status) {
        int message_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &message_len);
        char message[message_len];
        glGetShaderInfoLog(shader, message_len, NULL, message);
        glDeleteShader(shader);
        ERROR_RETURN(0, "Unable to compile shader\n Error: %s\n", message);
    }
    free(vert_shader_src.data);
    free(frag_shader_src.data);
    return shader;
}

uint32 _compile_shader(const void *shader_src, GLenum shader_type) {
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

