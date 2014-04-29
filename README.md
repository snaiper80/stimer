stimer
======

Simple Timing-Wheel Timer

Base Algorithm
---
Simple Timing Wheel
http://www.cse.wustl.edu/~cdgill/courses/cs6874/TimingWheels.ppt
http://gborah.wordpress.com/2011/08/01/timeout-management-using-hashed-timing-wheels/

Notice
---
* (Current) NOT Thread-safe code
* NO Timer-scheduling code (maybe you needs scheduling thread)

Supported Mode
---
* One-shot : Called once only
* Periodic : Called repeatedly

Compiling Test Code
---
```
$) make
```

Usage
---
```c
void timer_callback(stimer_t *timer, void *user_data)
{
    time_t origin = stimer_get_origin_time(timer);
    double diff   = difftime(time(NULL), origin);
    printf("[+ %02d] callback function was invoked: timer = %p, user data = %p\n", (int)diff, timer, user_data);
}
...
stimer_t *timer = stimer_create(0);

stimer_add_entry(timer, 2,  STIMER_ONESHOT_MODE, timer_callback, (void *)1);
stimer_add_entry(timer, 10, STIMER_PERIODIC_MODE, timer_callback, (void *)2);
stimer_add_entry(timer, 20, STIMER_ONESHOT_MODE, timer_callback, (void *)3);
stimer_add_entry(timer, 65, STIMER_ONESHOT_MODE, timer_callback, (void *)4);

printf("stimer started\n");

while (total_tick--)
{
    stimer_schedule_on_tick(timer);
    msleep(20);
}

printf("stimer ended\n");

stimer_destroy(&timer);
```

Result
```
stimer started
[+ 02] callback function was invoked: timer = 0x83c010, user data = 0x1
[+ 10] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 20] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 20] callback function was invoked: timer = 0x83c010, user data = 0x3
[+ 30] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 40] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 50] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 60] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 65] callback function was invoked: timer = 0x83c010, user data = 0x4
[+ 70] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 80] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 90] callback function was invoked: timer = 0x83c010, user data = 0x2
[+ 100] callback function was invoked: timer = 0x83c010, user data = 0x2
stimer ended
```