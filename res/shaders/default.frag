#version 330 core

in vec2 v_uv;

out vec4 frag_color;

uniform vec4 color;
uniform sampler2D texture_id;

void main() {
    frag_color = texture(texture_id, v_uv) * color;
}
