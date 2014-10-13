#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <cust_locktest.h>



////////////// START CUSTOM LOCK TESTS

// test if the holder is always correct
int locktest_holder()
{
    kprintf("jo!");

    return 1;
}



// execute all tests in this testsuite
int
locktest_extended(int nargs, char **args)
{


    (void)nargs;
    (void)args;


    //locktest_holder();

    return 1;



}
