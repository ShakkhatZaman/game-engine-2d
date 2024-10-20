#include <SDL2/SDL.h>

#include "time.h"
#include "global.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void time_delay(uint32 time_ms);

void time_init(uint32 frame_rate) {
    timing.frame_rate = frame_rate;
    timing.frame_delay = (frame_rate == 0) ? 0 : 1000.0 / (float32) frame_rate;
}

void time_update(void) {
    timing.last = timing.now;
    timing.now = (float32) SDL_GetTicks64();
    timing.delta = (timing.now - timing.last) / 1000.0;

    if ((timing.now - timing.frame_last) >= 1000.0) {
        timing.frame_last = timing.now;
        timing.frame_rate = timing.frame_count;
        timing.frame_count = 0;
    }
}

void time_update_end(void) {
    timing.frame_time = ((float32) SDL_GetTicks64()) - timing.now;
    timing.frame_count++;
    if (timing.frame_delay > timing.frame_time)
        time_delay((uint32) (timing.frame_delay - timing.frame_time));
}

void time_delay(uint32 time_ms) {
#ifdef _WIN32
    Sleep(time_ms);
#else
    usleep(time_ms * 1000);
#endif
}
