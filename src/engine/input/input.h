#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

typedef enum input_key {
    KEY_LEFT = 0,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_ESCAPE,
    KEY_SHOOT
} Input_key;

typedef enum key_state {
    KEY_UNPRESSED,
    KEY_PRESSED,
    KEY_HELD
} Key_state;

void input_update(void);

#endif // !INPUT_H
