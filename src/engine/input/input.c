#include "input.h"
#include "../global.h"

static void update_key_state(uint8 new_state, Key_state *state_ptr);
static void update_all_keys(Key_state new_state, int key);

void input_update(void) {
    const uint8 *keyboard_state = SDL_GetKeyboardState(NULL);

    update_key_state(keyboard_state[config.keybinds[KEY_LEFT]], &keys[KEY_LEFT]);
    update_key_state(keyboard_state[config.keybinds[KEY_RIGHT]], &keys[KEY_RIGHT]);
    update_key_state(keyboard_state[config.keybinds[KEY_UP]], &keys[KEY_UP]);
    update_key_state(keyboard_state[config.keybinds[KEY_DOWN]], &keys[KEY_DOWN]);
    update_key_state(keyboard_state[config.keybinds[KEY_ESCAPE]], &keys[KEY_ESCAPE]);
    update_key_state(keyboard_state[config.keybinds[KEY_SHOOT]], &keys[KEY_SHOOT]);
}
// checks from the config.keybinds array in global.c set by load_controls

static void update_key_state(uint8 new_state, Key_state *state_ptr) {
    if (new_state ==  KEY_PRESSED)
        *state_ptr = (*state_ptr != KEY_UNPRESSED) ? KEY_HELD : KEY_PRESSED;
    else
        *state_ptr = KEY_UNPRESSED;
}

