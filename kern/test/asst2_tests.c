#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <synch_queue.h>
#include <synch_heap.h>
#include <synch_hashtable.h>
#include <wchan.h>
#include <test.h>
#include <pid.h>
#include <asst2_tests.h>
#include <limits.h>



/* PID tests */

// do an initial test: acquire - release - acquire
int test_minimal_acquire_release_acquire_counter(){

    kprintf("\n****** testing sequential counter acquire - release - acquire *******\n");

    int new_id = get_new_process_id();
    release_process_id(new_id);

    // the queue should have gotten this item, check if it is the same
    KASSERT(get_new_process_id() == new_id);

    // when acquiring a new item now, it should be larger
    KASSERT(get_new_process_id() > new_id);

    kprintf("\n******  done testing sequential counter acquire - release - acquire *******\n");

    return 0;

}


// test if PID counter reaches upper limit, get_new_process_id returns -1
int test_pid_upper_limit_counter(){
    kprintf("\n****** testing PID upper limit *******\n");

    // what is the current PID counter, probably 2
    kprintf("\nCurrent PID counter: %d \n", PID_counter);

    int last_pid = 0;

    bool pid_valid = true;

    // get new pid until we are full
    while(pid_valid){
        int c_pid = get_new_process_id();

        if(c_pid == -1){
            break;
        }
        last_pid = c_pid;           
    }

    // last pid should be the limit
    if(last_pid != __PID_MAX){
        kprintf("last PID was not PID_MAX, instead: %d\n", last_pid);
        return 1;
    }else{
        kprintf("last PID was %d\n",last_pid);
    }

    kprintf("\n****** done testing PID upper limit *******\n");

    return 0;

}

// test release of pids
int test_pid_release(){
    kprintf("\n****** testing PID release *******\n");

    // assert that PID should be full by now
    KASSERT(get_new_process_id() == -1);


    int pid_to_release = 10000; //10.000

    release_process_id(pid_to_release);


    // acquire new one, should now be 10.000 again
    KASSERT(get_new_process_id() == 10000);

    

    kprintf("\n****** done testing PID release *******\n");

    return 0;
}


// test upper limit of queue
int test_pid_upper_limit_queue(){
    kprintf("\n****** testing PID upper limit queueu *******\n");

    KASSERT(get_new_process_id() == -1);

    // release 1000 ids
    for(int i = 1000; i < 100 + 1000; i++){
        release_process_id(i);
    }

    // reaquire 1000 ids
    for(int i = 1000; i < 100 + 1000; i++){
        KASSERT(get_new_process_id() != -1);
    }


    kprintf("\n****** done testing PID upper limit queueu *******\n");

    return 0;
}


// do an final test: acquire - release - acquire
int test_minimal_acquire_release_acquire_queue(){

    kprintf("\n****** testing sequential queue acquire - release - acquire *******\n");

    // release one id
    release_process_id(100);

    int new_id = get_new_process_id();
    KASSERT(new_id != -1);
    release_process_id(new_id);

    // this is after the queue kicks in, so we should get the same PID
    KASSERT(get_new_process_id()  == new_id);

    kprintf("\n******  done testing sequential queue acquire - release - acquire *******\n");

    return 0;

}


// release ids in a range
void release_ids(int from, int to){
    for(int i = from; i < to; i++){
        release_process_id(i);
    }
}





// rudimentary test synch hashtable
int test_synch_hashtable(){

    kprintf("\n****** testing synch hashmap *******\n");

    struct synch_hashtable *ht;

    ht = synch_hashtable_create();

    int val = 26;

    char key3[] = "felix";

    kprintf("test add \n");
    synch_hashtable_add(ht, key3, 5, &val);

    kprintf("test find \n");
    int res = *(int*)synch_hashtable_find(ht, key3, 5);
    kprintf("result %d \n", res);

    kprintf("test empty \n");
    KASSERT(synch_hashtable_isempty(ht) == 0);

    kprintf("test size \n");
    KASSERT(synch_hashtable_getsize(ht)==1);


    kprintf("test remove \n");
    synch_hashtable_remove(ht, key3, 5);

    KASSERT(synch_hashtable_isempty(ht) == 1);


    synch_hashtable_destroy(ht);

    kprintf("\n****** done testing synch hashmap *******\n");

    return 0;
}



int asst2_tests(int nargs, char **args){
    (void) nargs;
    (void) args;

    kprintf("starting tests for PID");
    // DO NOT CHANGE THE ORDER HERE!
    KASSERT(test_minimal_acquire_release_acquire_counter() == 0);
    KASSERT(test_pid_upper_limit_counter() == 0);
    KASSERT(test_pid_release() == 0);    
    KASSERT(test_minimal_acquire_release_acquire_queue() == 0);    


    test_synch_hashtable();


    release_ids(10000,30000);

    return 0;
}
