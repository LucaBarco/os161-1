#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <wchan.h>
#include <test.h>

static struct lock *testlock;
static struct cv *testcv;
static struct semaphore *donesem;
static volatile unsigned long testval;

static void initcv(void)
{
    testval = 0;

    if (testlock==NULL) {
        testlock = lock_create("testlock");
        if (testlock == NULL) {
            panic("synchtest: lock_create failed\n");
        }
    }
    if (testcv==NULL) {
        testcv = cv_create("testlock");
        if (testcv == NULL) {
            panic("synchtest: cv_create failed\n");
        }
    }
    if (donesem==NULL) {
        donesem = sem_create("donesem", 0);
        if (donesem == NULL) {
            panic("synchtest: sem_create failed\n");
        }
    }
}

static int cvtest_thread(void *junk, unsigned long num)
{
    (void)junk;
    lock_acquire(testlock);
    V(donesem);
    while(testval != num)
        cv_wait(testcv, testlock);
    V(donesem);
    lock_release(testlock);
    return 0;
}

//single thread wait/signal
int cvtest_wait(int nargs, char **args)
{
    (void)nargs;
    (void)args;
    int result;
    initcv();
    kprintf("Starting CV wait test...\n");
    
    //create a thread that'll just wait until testval == 1
    result = thread_fork("cvtest", NULL, NULL, cvtest_thread, NULL, 1);
    if (result) {
        panic("cvtest: thread_fork failed: %s\n",
              strerror(result));
    }
    P(donesem);
    //other thread is waiting
    
    lock_acquire(testlock);
        //other thread should be in the wchan's waitlist
        //KASSERT(!wchan_isempty(testcv->cv_wchan, &testcv->cv_lock));
        
        //increment testval to wake the thread
        testval++;
        cv_signal(testcv, testlock);
    lock_release(testlock);
    P(donesem);
    
    //thread should've woken up
    //KASSERT(wchan_isempty(testcv->cv_wchan, &testcv->cv_lock));
    
    kprintf("CV wait test done\n");
    return 0;
}

//cv_signal only wakes 1 thread
int cvtest_signal(int nargs, char **args)
{
    (void)nargs;
    (void)args;
    int result;
    initcv();
    kprintf("Starting CV signal test...\n");
    
    //create threads waiting for testval == 1
    result = thread_fork("cvtest", NULL, NULL, cvtest_thread, NULL, 1);
    if (result) {
        panic("cvtest: thread_fork failed: %s\n",
              strerror(result));
    }
    result = thread_fork("cvtest", NULL, NULL, cvtest_thread, NULL, 1);
    if (result) {
        panic("cvtest: thread_fork failed: %s\n",
              strerror(result));
    }
    P(donesem);
    P(donesem);
    //threads are now waiting
    
    //increment testval so either could wake, but only signal one
    lock_acquire(testlock);
        testval++;
        cv_signal(testcv, testlock);
    lock_release(testlock);
    P(donesem);
    
    lock_acquire(testlock);
        //should still be one thread waiting
        //KASSERT(!wchan_isempty(testcv->cv_wchan, &testcv->cv_lock));
        
        //signal the other thread
        cv_signal(testcv, testlock);
    lock_release(testlock);
    P(donesem);
    
    //all threads should've woken up
    //KASSERT(wchan_isempty(testcv->cv_wchan, &testcv->cv_lock));
    
    kprintf("CV signal test done\n");
    return 0;
}

//cv_broadcast wakes all threads
int cvtest_broadcast(int nargs, char **args)
{
    (void)nargs;
    (void)args;
    int result;
    initcv();
    kprintf("Starting CV broadcast test...\n");
    
    //create two threads waiting on testval == 1
    result = thread_fork("cvtest", NULL, NULL, cvtest_thread, NULL, 1);
    if (result) {
        panic("cvtest: thread_fork failed: %s\n",
              strerror(result));
    }
    result = thread_fork("cvtest", NULL, NULL, cvtest_thread, NULL, 1);
    if (result) {
        panic("cvtest: thread_fork failed: %s\n",
              strerror(result));
    }
    
    //create one thread waiting on testval == 2
    result = thread_fork("cvtest", NULL, NULL, cvtest_thread, NULL, 2);
    if (result) {
        panic("cvtest: thread_fork failed: %s\n",
              strerror(result));
    }
    
    P(donesem);
    P(donesem);
    P(donesem);
    //threads are now waiting
    
    //increment testval and then broadcast the change so two threads should wake up and call V()
    lock_acquire(testlock);
        testval++;
        cv_broadcast(testcv, testlock);
    lock_release(testlock);
    P(donesem);
    P(donesem);
    
    lock_acquire(testlock);
        //should still be one thread waiting
        //KASSERT(!wchan_isempty(testcv->cv_wchan, &testcv->cv_lock));
        
        //increment again, waking the last thread
        testval++;
        cv_broadcast(testcv, testlock);
    lock_release(testlock);
    P(donesem);
    
    //the cv shouldn't have any waiting threads
    //KASSERT(wchan_isempty(testcv->cv_wchan, &testcv->cv_lock));
    
    kprintf("CV broadcast test done\n");
    return 0;
}

int asst1_tests(int nargs, char **args)
{
    KASSERT(cvtest_wait(nargs, args) == 0);
    KASSERT(cvtest_signal(nargs, args) == 0);
    KASSERT(cvtest_broadcast(nargs, args) == 0);
    return 0;
}
