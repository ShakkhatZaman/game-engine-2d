#ifndef TIME_H
#define TIME_H

#include "types.h"

typedef struct time_state {
    float32 delta, now, last;
    float32 frame_last, frame_delay, frame_time;
    uint32 frame_rate, frame_count;
} Time_state;

void time_init(uint32 frame_rate);
void time_update(void);
void time_update_end(void);

void time_delay(uint32 time_ms);

#endif // !TIME_H
