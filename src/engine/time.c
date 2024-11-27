#include <SDL2/SDL.h>

#include "time.h"
#include "global.h"
#include "list.h"
#include "utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static List *timer_list;

void time_init(uint32 frame_rate) {
    timing.frame_rate = frame_rate;
    timing.frame_delay = (frame_rate == 0) ? 0 : 1000.0 / (float32) frame_rate;
    timer_list = list_create(0, sizeof(Timer));
}

void time_update(void) {
    timing.last = timing.now;
    timing.now = (float32) SDL_GetTicks64();
    timing.delta = (timing.now - timing.last) / 1000.0;
    Timer *timer;
    for (uint64 i = 0; i < timer_list->len; i++) {
        timer = list_get(timer_list, i);
        if (timer->running) {
            float32 delta = timing.now - timer->current_time;
            timer->time_left -= delta;
            if (timer->time_left <= 0) {
                timer->complete = true;
                timer->time_left = 0;
                timer->running = false;
            }
            timer->current_time = timing.now;
        }
    }

    if ((timing.now - timing.frame_last) >= 1000.0) {
        timing.frame_last = timing.now;
        timing.frame_rate = timing.frame_count;
        timing.frame_count = 0;
    }
}

uint64 timer_create(float32 duration, bool start_now) {
    uint64 id = timer_list->len;
    for (uint64 i = 0; i < timer_list->len; i++) {
        Timer *timer = list_get(timer_list, i);
        if (!timer->active) {
            id = i;
            break;
        }
    }

    if (id == timer_list->len) {
        if (list_append(timer_list, &(Timer){0}) == -1) {
            ERROR_EXIT_PROGRAM("Unable to add to timer list\n");
        }
    }

    Timer *timer = list_get(timer_list, id);
    *timer = (Timer){
        .running = start_now, .complete = false, .active = true,
        .duration = duration, .time_left = duration, .current_time = (float32) SDL_GetTicks64()
    };

    return id;
}

void timer_start(uint64 timer_id) {
    Timer *timer = list_get(timer_list, timer_id);
    timer->running = true;
    timer->complete = false;
    timer->current_time = (float32) SDL_GetTicks64();
}

void timer_restart(uint64 timer_id) {
    Timer *timer = list_get(timer_list, timer_id);
    timer->running = true;
    timer->complete = false;
    timer->time_left = timer->duration;
    timer->current_time = (float32) SDL_GetTicks64();
}

bool timer_check_complete(uint64 timer_id) {
    Timer *timer = list_get(timer_list, timer_id);
    return timer->complete;
}

void timer_stop(uint64 timer_id) {
    Timer *timer = list_get(timer_list, timer_id);
    timer->running = false;
}

void timer_destroy(uint64 timer_id) {
    Timer *timer = list_get(timer_list, timer_id);
    timer->active = false;
}

void time_update_end(void) {
    timing.frame_time = ((float32) SDL_GetTicks64()) - timing.now;
    timing.frame_count++;
    if (timing.frame_delay > timing.frame_time)
        time_delay((uint32) (timing.frame_delay - timing.frame_time));
}

void time_exit(void) {
    list_delete(timer_list);
}

void time_delay(uint32 time_ms) {
#ifdef _WIN32
    Sleep(time_ms);
#else
    usleep(time_ms * 1000);
#endif
}
