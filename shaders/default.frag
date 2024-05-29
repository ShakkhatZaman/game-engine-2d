#version 330 core

out vec4 frag_color;

in vec2 v_uv;

uniform vec4 color;
uniform sampler2D texture_id;

void main() {
    frag_color = texture(texture_id, v_uv) * color;
}
