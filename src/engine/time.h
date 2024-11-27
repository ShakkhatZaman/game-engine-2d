#ifndef TIME_H
#define TIME_H

#include "types.h"

typedef struct time_state {
    float32 delta, now, last;
    float32 frame_last, frame_delay, frame_time;
    uint32 frame_rate, frame_count;
} Time_state;

typedef struct timer {
    bool running, complete, active;
    float32 duration, time_left, current_time;
} Timer;

void time_init(uint32 frame_rate);
void time_exit(void);
void time_update(void);
void time_update_end(void);

uint64 timer_create(float32 duration, bool start_now);
void timer_start(uint64 timer_id);
void timer_restart(uint64 timer_id);
bool timer_check_complete(uint64 timer_id);
void timer_stop(uint64 timer_id);
void timer_destroy(uint64 timer_id);

void time_delay(uint32 time_ms);

#endif // !TIME_H
