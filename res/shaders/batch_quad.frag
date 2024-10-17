#version 330 core

in vec2 v_uv;
in vec4 v_color;

flat in int v_texture_slot;

out vec4 out_color;

uniform sampler2D texture_slot_0;
uniform sampler2D texture_slot_1;
uniform sampler2D texture_slot_2;
uniform sampler2D texture_slot_3;
uniform sampler2D texture_slot_4;
uniform sampler2D texture_slot_5;
uniform sampler2D texture_slot_6;
uniform sampler2D texture_slot_7;

void main() {
    vec4 albedo = vec4(0);
    switch (v_texture_slot) {
        case 0 : albedo = texture(texture_slot_0, v_uv); break;
        case 1 : albedo = texture(texture_slot_1, v_uv); break;
        case 2 : albedo = texture(texture_slot_2, v_uv); break;
        case 3 : albedo = texture(texture_slot_3, v_uv); break;
        case 4 : albedo = texture(texture_slot_4, v_uv); break;
        case 5 : albedo = texture(texture_slot_5, v_uv); break;
        case 6 : albedo = texture(texture_slot_6, v_uv); break;
        case 7 : albedo = texture(texture_slot_7, v_uv); break;
    }

    out_color = albedo * v_color;
}
