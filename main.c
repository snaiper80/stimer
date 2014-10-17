#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "stimer.h"

//#define TEST_CANCEL_ALL_TIME_ENTRY
//#define TEST_CANCEL_ONE_TIME_ENTRY

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
    int                total_tick  = 5000;
    //int              total_tick  = 100;
    stimer_t          *timer       = stimer_create(0);
    stimer_entry_id_t  entry_id    = 0;

    UNUSED(argc);
    UNUSED(argv);

    stimer_schedule_entry(timer, 2,  STIMER_ONESHOT_MODE, timer_callback, (void *)1);
    stimer_schedule_entry(timer, 10, STIMER_PERIODIC_MODE, timer_callback, (void *)2);
    stimer_schedule_entry(timer, 20, STIMER_ONESHOT_MODE, timer_callback, (void *)3);

    entry_id = stimer_schedule_entry(timer, 65, STIMER_ONESHOT_MODE, timer_callback, (void *)4);

    printf("stimer started\n");

    while (total_tick--)
    {
        stimer_schedule_on_tick(timer);
        msleep(20);
        //sleep(1);

#ifdef TEST_CANCEL_ALL_TIME_ENTRY
        if (total_tick == 4000)
        {
            stimer_cancel_all_entries(timer);
            printf("cancel all timer.\n");
        }
#endif

#ifdef TEST_CANCEL_ONE_TIME_ENTRY
        if (total_tick == 4000)
        {
            printf("cancel 1 time entry.\n");
            stimer_cancel_entry(timer, entry_id);
        }
#endif
    }

    printf("stimer ended\n");

    stimer_destroy(&timer);
}