#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

typedef enum input_key {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_ESCAPE,
} Input_key;

typedef enum key_state {
    KEY_UNPRESSED,
    KEY_PRESSED,
    KEY_HELD
} Key_state;

typedef struct input_states {
    Key_state up, down, left, right;
    Key_state escape;
} Input_states;

void input_update(void);

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

#endif // !INPUT_H
