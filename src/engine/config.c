#include <stdbool.h>
#include <string.h>

#include "global.h"
#include "input/input.h"
#include "utils.h"
#include "config.h"
#include "io/io.h"

static const char config_file_name[] = "./config.ini";

static bool load_config(void);
static void load_controls(const char *config_buffer);
static char *config_get_key_name(const char *buffer, const char *control_name);
static void config_set_key_bind(Input_key key, const char *key_name);

char temp_keyname_buffer[20] = {0};

void config_init(void) {
    if (load_config()) return;
 
    const char DEFAULT_CONFIG[] = 
        "[controls]\n"
        "left = A\n"
        "right = D\n"
        "up = W\n"
        "down = S\n"
        "escape = Escape\n"
        "\n";

    write_file(config_file_name, strlen(DEFAULT_CONFIG), DEFAULT_CONFIG);
    fprintf(stderr, "Creating config file : %s\n", config_file_name);
    if (!load_config()) {
        ERROR_EXIT_PROGRAM("Unable to load or create config file\n");
    }
}

static bool load_config(void) {
    File config_file = read_file(config_file_name);
    if (!config_file.is_valid) {
        return false;
    }

    load_controls(config_file.data);

    free(config_file.data);
    return true;
}

// Sets the config.keybinds array with all keybinds
static void load_controls(const char *config_buffer) {
    config_set_key_bind(KEY_LEFT, config_get_key_name(config_buffer, "left"));
    config_set_key_bind(KEY_RIGHT, config_get_key_name(config_buffer, "right"));
    config_set_key_bind(KEY_UP, config_get_key_name(config_buffer, "up"));
    config_set_key_bind(KEY_DOWN, config_get_key_name(config_buffer, "down"));
    config_set_key_bind(KEY_ESCAPE, config_get_key_name(config_buffer, "escape"));
}

// returns the ***key enum*** from the keyname
static char *config_get_key_name(const char *buffer, const char *control_name) {
    if (!strstr(buffer, "[controls]")) {
        ERROR_EXIT_PROGRAM("Unable to parse config. try deleting %s\n", config_file_name);
    }
    char *line = strstr(buffer, control_name);
    if (!line) {
        ERROR_EXIT_PROGRAM("Unable to parse config. try deleting %s\n", config_file_name);
    }

    uint64 length = strlen(line);
    char *end = line + length, *current = line, *tmp_ptr = temp_keyname_buffer;

    while (*current != '=' && current != end) current++; // skip all chars before '='
    current++; // skip '='
    while (*current == ' ') current++; // skip whitespace

    while (*current != ' ' && *current != '\n' && *current != '\0' && current != end)
        *tmp_ptr++ = *current++; //copy all chars after the '=' and whitespace until '\n'

    *tmp_ptr = '\0';
    return temp_keyname_buffer;
}

static void config_set_key_bind(Input_key key, const char *key_name) {
    SDL_Scancode scancode = SDL_GetScancodeFromName(key_name);
    if (scancode == SDL_SCANCODE_UNKNOWN) {
        ERROR_EXIT_PROGRAM("Invalid keyname used in config file : %s\n", key_name);
    }
    config.keybinds[key] = scancode;
}
