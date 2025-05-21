#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "stimer.h"

//#define TEST_CANCEL_ALL_TIME_ENTRY
//#define TEST_CANCEL_ONE_TIME_ENTRY

#define UNUSED(x) (void)(x)

// Forward declaration for timer_callback, used by tests
static void timer_callback(stimer_t *timer, void *user_data);

static void msleep (unsigned int ms)
{
    int microsecs;
    struct timeval tv;
    microsecs = ms * 1000;
    tv.tv_sec  = microsecs / 1000000;
    tv.tv_usec = microsecs % 1000000;
    select (0, NULL, NULL, NULL, &tv);
}

// --- Test Functions ---

void test_negative_diff_in_get_slot() {
    printf("Running test_negative_diff_in_get_slot...\n");
    stimer_t *test_timer = stimer_create(0); 
    if (!test_timer) {
        printf("Test FAILED (test_negative_diff_in_get_slot): stimer_create failed.\n");
        return;
    }
    unsigned int delay_seconds = 1; 
    stimer_entry_id_t entry_id = stimer_schedule_entry(test_timer, delay_seconds, STIMER_ONESHOT_MODE, timer_callback, (void *)0x100);

    if (entry_id != 0) {
        printf("Test PASSED (test_negative_diff_in_get_slot): stimer_schedule_entry returned a valid ID (%lu).\n", (unsigned long)entry_id);
        stimer_cancel_entry(test_timer, entry_id); 
    } else {
        printf("Test FAILED (test_negative_diff_in_get_slot): stimer_schedule_entry returned 0.\n");
    }
    stimer_destroy(&test_timer);
    printf("test_negative_diff_in_get_slot completed.\n\n");
}

void test_create_destroy_empty_timer() {
    printf("Running test_create_destroy_empty_timer...\n");
    stimer_t *timer = stimer_create(0);
    if (!timer) {
        printf("Test FAILED (test_create_destroy_empty_timer): stimer_create failed.\n");
        return;
    }
    stimer_destroy(&timer); 
    if (timer == NULL) {
        printf("Test PASSED (test_create_destroy_empty_timer): Timer created and destroyed successfully.\n");
    } else {
        printf("Test FAILED (test_create_destroy_empty_timer): Timer pointer was not NULL after destroy.\n");
    }
    printf("test_create_destroy_empty_timer completed.\n\n");
}

// --- Original Functions ---

void timer_callback(stimer_t *timer, void *user_data)
{
    time_t origin = stimer_get_origin_time(timer);
    double diff   = difftime(time(NULL), origin);
    printf("[+ %02d] callback function was invoked: timer = %p, user data = %p\n", (int)diff, timer, user_data);
}

int main(int argc, char *argv[])
{
    // Run tests first
    printf("--- Running Tests ---\n");
    test_negative_diff_in_get_slot();
    test_create_destroy_empty_timer();
    printf("--- Tests Completed ---\n\n");

    // Original demo code
    printf("--- Starting Original Demo ---\n");
    int                total_tick  = 100; // Reduced ticks for faster execution
    stimer_t          *timer       = stimer_create(0);
    stimer_entry_id_t  entry_id    = 0;

    UNUSED(argc);
    UNUSED(argv);

    if (!timer) { 
        printf("[Original Demo] Failed to create timer.\n");
        return 1;
    }

    stimer_schedule_entry(timer, 2,  STIMER_ONESHOT_MODE, timer_callback, (void *)0xA001);
    stimer_schedule_entry(timer, 3, STIMER_PERIODIC_MODE, timer_callback, (void *)0xA002); 
    stimer_schedule_entry(timer, 4, STIMER_ONESHOT_MODE, timer_callback, (void *)0xA003); 
    entry_id = stimer_schedule_entry(timer, 5, STIMER_ONESHOT_MODE, timer_callback, (void *)0xA004); 
    UNUSED(entry_id); // To suppress unused warning if original #ifdefs are not used.

    printf("[Original Demo] stimer started. Running for %d ticks (~%d seconds).\n", total_tick, total_tick * 20 / 1000);

    for(int i = 0; i < total_tick; ++i)
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

    printf("[Original Demo] stimer ended.\n");

    stimer_destroy(&timer);
    printf("--- Original Demo Finished ---\n");

    return 0;
}