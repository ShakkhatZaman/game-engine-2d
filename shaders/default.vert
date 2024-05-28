#version 330 core

layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec2 in_uv;

out vec2 v_uv;

uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * model * in_pos;
    v_uv = in_uv;
}
