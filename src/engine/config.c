#include <stdbool.h>
#include <string.h>

#include "global.h"
#include "input.h"
#include "utils.h"
#include "config.h"
#include "io/io.h"

static const char *DEFAULT_CONFIG = 
    "[controls]\n"
    "left = A\n"
    "right = D\n"
    "up = W\n"
    "down = S\n"
    "escape = Escape\n"
    "\n";

static bool load_config(void);
static void load_controls(const char *config_buffer);
static int config_get_glfw_key(const char *buffer, const char *control_name);
static void config_set_key_bind(Input_key key, int glfw_key);

char temp_keyname_buffer[20] = {0};

void config_init(void) {
    if (load_config()) return;
    
    write_file("./config.ini", strlen(DEFAULT_CONFIG), DEFAULT_CONFIG);
    if (!load_config()) {
        ERROR_EXIT_PROGRAM("Unable to load or create config file\n");
    }
}

static bool load_config(void) {
    File config_file = read_file("./config.ini");
    if (!config_file.is_valid) {
        return false;
    }

    load_controls(config_file.data);

    free(config_file.data);
    return true;
}

// Sets the config.keybinds array with all keybinds
static void load_controls(const char *config_buffer) {
    config_set_key_bind(KEY_LEFT, config_get_glfw_key(config_buffer, "left"));
    config_set_key_bind(KEY_RIGHT, config_get_glfw_key(config_buffer, "right"));
    config_set_key_bind(KEY_UP, config_get_glfw_key(config_buffer, "up"));
    config_set_key_bind(KEY_DOWN, config_get_glfw_key(config_buffer, "down"));
    config_set_key_bind(KEY_ESCAPE, config_get_glfw_key(config_buffer, "escape"));
}

// returns the ***key enum*** from the keyname
static int config_get_glfw_key(const char *buffer, const char *control_name) {
    char *line = strstr(buffer, control_name);
    if (!line) {
        ERROR_EXIT_PROGRAM("Unable to parse config. try deleting config.ini\n");
    }

    uint64 length = strlen(line);
    char *end = line + length, *current = line, *tmp_ptr = temp_keyname_buffer;

    while (*current != '=' && current != end) current++; // skip all chars before '='
    current++; // skip '='
    while (*current == ' ') current++; // skip whitespace

    while (*current != ' ' && *current != '\n' && *current != '\0' && current != end)
        *tmp_ptr++ = *current++; //copy all chars after the '=' and whitespace until '\n'

    *tmp_ptr = '\0';

    if (!strcmp(control_name, "left")) return GLFW_KEY_LEFT;
    if (!strcmp(control_name, "right")) return GLFW_KEY_RIGHT;
    if (!strcmp(control_name, "up")) return GLFW_KEY_UP;
    if (!strcmp(control_name, "down")) return GLFW_KEY_DOWN;
    else return GLFW_KEY_ESCAPE;
}

void config_set_key_bind(Input_key key, int glfw_key) {
    config.keybinds[key] = glfw_key;
}
