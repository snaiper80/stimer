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
#include "stimer.h"

// includes - standard library
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sys/queue.h>

// constants
#define ONE_MINUTES_TO_SECONDS          (60)           
#define DEFAULT_TOTAL_TIME_SLOT         (60)


// macros
#ifndef ST_SAFE_FREE 
#define ST_SAFE_FREE(p) { if (p) { free((p)); (p) = NULL; } } 
#endif /* ST_SAFE_FREE */


#define TM_ENTRIES                LIST_ENTRY(stimer_entry)
#define TM_ENTRIES_HEAD           LIST_HEAD(stimer_entries, stimer_entry)


// NOTE: Fixed compile error in linux
#ifndef LIST_FOREACH_SAFE
#define LIST_FOREACH_SAFE(var, head, field, tvar)                       \
        for ((var) = LIST_FIRST(head);                                  \
            (var) && ((tvar) = LIST_NEXT(var, field), 1);               \
            (var) = (tvar))
#endif /* ST_SAFE_FREE */



// types
struct stimer_entry
{
    unsigned int                  id;
    time_t                        started;
    time_t                        expired;
    unsigned int                  delay;

    stimer_mode_t                 mode;

    stimer_expired_callback_t     callback;
    void                         *user_data;

    TM_ENTRIES                    entries;
};

struct tm_slot
{ 
    stimer_t                     *timer;
    TM_ENTRIES_HEAD               entries_head;
};
typedef struct tm_slot tm_slot_t;

struct stimer
{
    unsigned int                  timeslot;
    unsigned int                  slot_interval_seconds;
    tm_slot_t                   **slots; 

    time_t                        origin;
    unsigned int                  next_id;
};


/* Private */
static int get_slot(stimer_t *timer, time_t now, int delay)
{
    int     slot_id;
    time_t  total_time;
    double  diff;

    if (now == 0)
        now = time(NULL);

    total_time = now + delay;
    diff       = difftime(timer->origin, total_time);
    slot_id    = (int)(diff / timer->slot_interval_seconds) % timer->timeslot;

    return slot_id;
} 

static void stimer_entry_set(stimer_entry_t            *entry, 
                             unsigned int               entry_id,
                             time_t                     started,
                             time_t                     expired,
                             unsigned int               delay,
                             stimer_mode_t              mode,
                             stimer_expired_callback_t  callback,
                             void                      *user_data)
{
    if (entry == NULL)
        return ;

    entry->id        = entry_id;
    entry->started   = started;
    entry->expired   = expired;
    entry->delay     = delay;
    entry->mode      = mode;
    entry->callback  = callback;
    entry->user_data = user_data;
}

static void stimer_entry_reset(stimer_entry_t *entry)
{
    stimer_entry_set(entry, 0, 0, 0, 0, STIMER_ONESHOT_MODE, NULL, NULL);
}

static stimer_entry_t * tm_slot_insert_entry_at_head(tm_slot_t                  *slot, 
                                                     unsigned int                entry_id,
                                                     time_t                      started, 
                                                     unsigned int                delay, 
                                                     stimer_mode_t               mode, 
                                                     stimer_expired_callback_t   callback, 
                                                     void                       *user_data)
{
    stimer_entry_t *new_entry = (stimer_entry_t *)malloc(sizeof(stimer_entry_t));
    stimer_entry_reset(new_entry);

    LIST_INSERT_HEAD(&slot->entries_head, new_entry, entries);

    stimer_entry_set(new_entry, 
                     entry_id,
                     started,
                     started + delay,
                     delay, 
                     mode,
                     callback, 
                     user_data);

    return new_entry;
}

static void stimer_entry_make_next_expired(tm_slot_t *slot, stimer_entry_t *entry)
{
    time_t          now;
    tm_slot_t      *new_slot  = NULL;
    stimer_entry_t *new_entry = NULL;
    stimer_t       *timer     = NULL;

    if (entry == NULL)
        return ;

    if (entry->mode != STIMER_PERIODIC_MODE)
        return ;

    timer     = slot->timer;
    now       = time(NULL);
    new_slot  = timer->slots[get_slot(timer, now, entry->delay)];

    new_entry = tm_slot_insert_entry_at_head(new_slot, 
                                             entry->id,
                                             now, 
                                             entry->delay, 
                                             entry->mode, 
                                             entry->callback, 
                                             entry->user_data);
    new_entry->started = entry->started;
}

static void tm_slot_process_timeout(tm_slot_t *slot)
{
    time_t          now;
    double          diff;
    stimer_t       *timer      = NULL;
    stimer_entry_t *entry      = NULL;
    stimer_entry_t *temp_entry = NULL;

    if (slot == NULL)
        return ;

    timer = slot->timer;

    LIST_FOREACH_SAFE(entry, &slot->entries_head, entries, temp_entry) 
    {
        if (entry->callback == NULL || 
            entry->started  == 0    ||
            entry->expired  == 0)
        {
            break;
        }

        now   = time(NULL);
        diff  = fabs(difftime(entry->expired, now));

        // expired or up to time
        if (now > entry->expired || diff < timer->slot_interval_seconds)
        {
            // slot has expired entry
            if (entry->callback)
                entry->callback(timer, entry->user_data);
            
            if (entry->mode == STIMER_PERIODIC_MODE)
                stimer_entry_make_next_expired(slot, entry);
            
            LIST_REMOVE(entry, entries);
            ST_SAFE_FREE(entry);
        }
        else
        {
            // entry is valid, so we passed.
        }
    }
}

/* Public */
stimer_t * stimer_create(int timeslot)
{
    stimer_t *new_timer = NULL;
    int       i         = 0;

    if (timeslot <= 0) 
        timeslot = DEFAULT_TOTAL_TIME_SLOT;
    
    new_timer                        = (stimer_t *)malloc(sizeof(*new_timer));
    new_timer->timeslot              = timeslot;
    new_timer->slot_interval_seconds = (ONE_MINUTES_TO_SECONDS / timeslot);
    new_timer->slots                 = (tm_slot_t **)malloc(timeslot * sizeof(tm_slot_t *));
    new_timer->next_id               = 1;

    for (i = 0; i < timeslot; ++i)
    {
        tm_slot_t *s = (tm_slot_t *)malloc(sizeof(tm_slot_t));
        assert(s != NULL);
        
        s->timer     = new_timer;
        LIST_INIT(&s->entries_head);

        new_timer->slots[i] = s;
    }

    new_timer->origin = time(NULL);
    
    return new_timer;
}

time_t stimer_get_origin_time(stimer_t *timer)
{
    if (timer != NULL)
        return timer->origin;
    else
        return 0;
}

stimer_entry_t * stimer_schedule_entry(stimer_t *timer, unsigned int delay, stimer_mode_t mode, stimer_expired_callback_t callback, void *user_data)
{
    time_t      now;
    tm_slot_t  *slot  = NULL;

    if (timer == NULL)
        return NULL;

    now   = time(NULL);
    slot  = timer->slots[get_slot(timer, now, delay)];
    assert(slot);

    return tm_slot_insert_entry_at_head(slot, timer->next_id++, now, delay, mode, callback, user_data);
}

void stimer_cancel_entry(stimer_t *timer, stimer_entry_t *entry)
{
    if (timer == NULL || entry == NULL)
        return ;

    LIST_REMOVE(entry, entries);
    ST_SAFE_FREE(entry);
}

void stimer_cancel_all_entries(stimer_t *timer)
{
    unsigned int     i;
    stimer_entry_t  *e1 = NULL, *e2 = NULL;

    if (timer == NULL)
        return ;

    for (i = 0; i < timer->timeslot; ++i)
    {
        tm_slot_t *slot = timer->slots[i];
        if (slot == NULL)
            continue;
        
        // cleanup list
        e1 = LIST_FIRST(&slot->entries_head);
        while (e1 != NULL) 
        {
            e2 = LIST_NEXT(e1, entries);
            ST_SAFE_FREE(e1);

            e1 = e2;
        }
        LIST_INIT(&slot->entries_head);
    }
}

void stimer_schedule_on_tick(stimer_t *timer)
{
    int         current_slot_id = 0;
    tm_slot_t  *current_slot    = NULL;

    if (timer == NULL)
        return ;

    current_slot_id = get_slot(timer, 0, 0);
    current_slot    = timer->slots[current_slot_id];

    tm_slot_process_timeout(current_slot);
}

void stimer_destroy(stimer_t **timer)
{
    unsigned int     i;
    stimer_entry_t  *e1 = NULL, *e2 = NULL;

    if (timer == NULL || *timer == NULL)
        return ;

    for (i = 0; i < (*timer)->timeslot; ++i)
    {
        tm_slot_t *slot = (*timer)->slots[i];
        if (slot == NULL)
            continue;
        
        // cleanup list
        e1 = LIST_FIRST(&slot->entries_head);
        while (e1 != NULL) 
        {
            e2 = LIST_NEXT(e1, entries);
            ST_SAFE_FREE(e1);

            e1 = e2;
        }
        LIST_INIT(&slot->entries_head);

        // cleanup slot
        ST_SAFE_FREE(slot);
    }

    ST_SAFE_FREE((*timer)->slots); 
    ST_SAFE_FREE(*timer);
}
