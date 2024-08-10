#version 330 core

in vec2 v_uv;
in vec4 v_color;

out vec4 out_color;

uniform sampler2D texture_id;

void main() {
    out_color = texture(texture_id, v_uv) * v_color;
}
