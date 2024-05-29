#include "GLFW/glfw3.h"

#include "time.h"
#include "global.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static void delay(uint32 time_ms);

void time_init(uint32 frame_rate) {
    timing.frame_rate = frame_rate;
    timing.frame_delay = 1000.0 / (float32) frame_rate;
}

void time_update(void) {
    timing.last = timing.now;
    timing.now = (float32) glfwGetTime() * 1000.0;
    timing.delta = (timing.now - timing.last) / 1000.0;

    if ((timing.now - timing.frame_last) >= 1000.0) {
        timing.frame_last = timing.now;
        timing.frame_rate = timing.frame_count;
        timing.frame_count = 0;
    } 
}

void time_update_end(void) {
    timing.frame_time = ((float32) glfwGetTime() * 1000.0) - timing.now;
    timing.frame_count++;
    if (timing.frame_delay > timing.frame_time)
        delay((uint32) (timing.frame_delay - timing.frame_time));

}

static void delay(uint32 time_ms) {
#ifdef _WIN32
    Sleep(time_ms);
#else
    usleep(time_ms * 1000);
#endif
}
