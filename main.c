#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "stimer.h"

#define UNUSED(x) (void)(x)

static void msleep (unsigned int ms)
{
    int microsecs;
    struct timeval tv;
    microsecs = ms * 1000;
    tv.tv_sec  = microsecs / 1000000;
    tv.tv_usec = microsecs % 1000000;
    select (0, NULL, NULL, NULL, &tv);  
}

void timer_callback(stimer_t *timer, void *user_data)
{
    time_t origin = stimer_get_origin_time(timer);
    double diff   = difftime(time(NULL), origin);
    printf("[+ %02d] callback function was invoked: timer = %p, user data = %p\n", (int)diff, timer, user_data);
}

int main(int argc, char *argv[])
{
    int       total_tick = 5000;
    //int       total_tick = 100;
    stimer_t *timer      = stimer_create(0);

    UNUSED(argc);
    UNUSED(argv);

    stimer_add_entry(timer, 2,  STIMER_ONESHOT_MODE, timer_callback, (void *)1);
    stimer_add_entry(timer, 10, STIMER_PERIODIC_MODE, timer_callback, (void *)2);
    stimer_add_entry(timer, 20, STIMER_ONESHOT_MODE, timer_callback, (void *)3);
    stimer_add_entry(timer, 65, STIMER_ONESHOT_MODE, timer_callback, (void *)4);

    printf("stimer started\n");

    while (total_tick--)
    {
        stimer_schedule_on_tick(timer);
        msleep(20);
        //sleep(1);
    }

    printf("stimer ended\n");

    stimer_destroy(&timer);
}