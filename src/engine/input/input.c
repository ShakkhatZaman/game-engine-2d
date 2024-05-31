#include "input.h"
#include "../global.h"

static void update_key_state(uint8 new_state, Key_state *state_ptr);
static void update_all_keys(Key_state new_state, int key);

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
        update_all_keys(KEY_PRESSED, key);
    else 
        update_all_keys(KEY_UNPRESSED, key);
}
// checks from the config.keybinds array in global.c set by load_controls
static void update_all_keys(Key_state new_state, int key) {
    if (key == config.keybinds[KEY_UP]) {
        update_key_state(new_state, &keys.up);
    }
    if (key == config.keybinds[KEY_DOWN]) {
        update_key_state(new_state, &keys.down);
    }
    if (key == config.keybinds[KEY_LEFT]) {
        update_key_state(new_state, &keys.left);
    }
    if (key == config.keybinds[KEY_RIGHT]) {
        update_key_state(new_state, &keys.right);
    }
    if (key == config.keybinds[KEY_ESCAPE]) {
        update_key_state(new_state, &keys.escape);
    }
}

static void update_key_state(uint8 new_state, Key_state *state_ptr) {
    if (new_state ==  KEY_PRESSED)
        *state_ptr = (*state_ptr != KEY_UNPRESSED) ? KEY_HELD : KEY_PRESSED;
    else
        *state_ptr = KEY_UNPRESSED;
}

