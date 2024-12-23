#version 330 core

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;
layout(location = 3) in int in_texture_slot;

out vec2 v_uv;
out vec4 v_color;

flat out int v_texture_slot;

uniform mat4x4 projection;

void main() {
    gl_Position = projection * vec4(in_pos, 0.0, 1.0);
    v_uv = in_uv;
    v_color = in_color;
    v_texture_slot = in_texture_slot;
}
