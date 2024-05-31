#ifndef GLOBAL_H
#define GLOBAL_H

#include "config.h"
#include "renderer/renderer.h"
#include "input/input.h"
#include "time.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

extern Renderer_state rendering_state;

extern Input_states keys;

extern Config_state config;

extern Time_state timing;

#endif // !GLOBAL_H
