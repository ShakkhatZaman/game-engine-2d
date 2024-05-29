#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

typedef struct config_state {
    int32 keybinds[5];
} Config_state;

void config_init(void);

#endif //CONFIG_H
