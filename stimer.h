/*
    Simple Timing-Wheel Timer

    The MIT License (MIT)

    Copyright (c) 2014 snaiper (Lee GiTack)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#ifndef STIMER_H
#define STIMER_H

// prerequisite headers
#include <time.h>

typedef struct stimer       stimer_t;
typedef struct stimer_entry stimer_entry_t;
typedef void (*stimer_expired_callback_t)(stimer_t *timer, void *user_data);
typedef enum {
    STIMER_ONESHOT_MODE = 0,
    STIMER_PERIODIC_MODE,
} stimer_mode_t;

stimer_t *       stimer_create(int timeslot);
time_t           stimer_get_origin_time(stimer_t *timer);
stimer_entry_t * stimer_schedule_entry(stimer_t *timer, unsigned int delay, stimer_mode_t mode, stimer_expired_callback_t callback, void *user_data);
void             stimer_cancel_entry(stimer_t *timer, stimer_entry_t *entry);
void             stimer_cancel_all_entries(stimer_t *timer);
void             stimer_schedule_on_tick(stimer_t *timer);
void             stimer_destroy(stimer_t **timer);

#endif
