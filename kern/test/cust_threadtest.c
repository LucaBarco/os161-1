#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <cust_threadtest.h>

#define NTHREADS      10

////////////// START CUSTOM THREAD TESTS

static
void
runthreadjointest(void)
{
	struct thread threads[NTHREADS];
	int exitValue;
	
	for(int i=0; i < NTHREADS; i++)
		thread_create(&threadsi, &go, i);

	for(int i=0; i < NTHREADS; i++)
	{
		exitValue = thread_join(threads[i]);
		kprintf("Thread %d returned with %d\n", i, exitValue);
	}

	kprintf("Main thread done.\n");
}

void go(int n)
{
	kprintf("Hello from thread %d\n", n);
	thread_exit(100 + n);
}


// execute all tests in this testsuite
int
threadtest4(int nargs, char **args)
{
    (void) nargs;
    (void) args;

    kprintf("Starting thread test 4 (join)...\n");
    runthreadjointest();
    kprintf("\nThread test 4 (join) done.\n");

    return 0;
}
