#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <cust_locktest.h>



#define NTHREADS      32


static struct semaphore *donesem; // is used to synchronize threads since we can not join jet
static struct lock *testlock;


////////////// START CUSTOM LOCK TESTS







static
int
fail(const char *msg, unsigned long num)
{
    kprintf("thread %lu: %s\n ",num, msg);
    kprintf("Test failed\n");

    lock_release(testlock);

    V(donesem);
    thread_exit(0);
}

// this is run by the locktest holder threads
static int locktest_holder_helper_function(void *junk, unsigned long num){

    (void)junk;


    // be sure that we are not the lock holder
    if(lock_do_i_hold(testlock)){
        fail("I should not hold the lock", num);
    }

    // try to acquire the lock
    lock_acquire(testlock);

        // i should have the lock now
        if(!lock_do_i_hold(testlock)){
            fail("I should hold the lock", num);
        }

        // yield for other pro cesses
        thread_yield();

        // check again for the holder
        if(!lock_do_i_hold(testlock)){
            fail("I should hold the lock (after yield)", num);
        }

    // release the lock
    lock_release(testlock);

    // be sure that we are not the lock holder
    if(lock_do_i_hold(testlock)){
        fail("I should not hold the lock (after release)", num);
    }



    V(donesem);

    return 0;
}


// tests if the holder is always correct
int locktest_holder()
{
    
    int result;

    

    
    kprintf("Starting lock holder test...\n");

    for (int i=0; i<NTHREADS; i++) {
        result = thread_fork("holder_test", NULL, NULL, locktest_holder_helper_function, NULL, i);
        if (result) {
            panic("locktest: thread_fork failed: %s\n",
                  strerror(result));
        }
    }
    for (int i=0; i<NTHREADS; i++) {
        P(donesem);
    }

    kprintf("Lock test done.\n");

    return 0;
}




// execute all tests in this testsuite
int
locktest_extended(int nargs, char **args)
{

    (void) nargs;
    (void) args;

    // initialize semaphore
    donesem = sem_create("lock_sem",0);

    // initialize lock
    testlock = lock_create("testlock");

    locktest_holder();

    return 0;



}
