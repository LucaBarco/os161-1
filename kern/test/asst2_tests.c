#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <synch_queue.h>
#include <synch_heap.h>
#include <wchan.h>
#include <test.h>
#include <pid.h>
#include <asst2_tests.h>



/* PID tests */
int test_pid_upper_limit(){
    // what is the current PID counter, probably 2

    kprintf("\n %d \n", PID_counter);

    return 0;
}




int asst2_tests(int nargs, char **args){
    (void) nargs;
    (void) args;

    kprintf("test");

    return 0;
}
