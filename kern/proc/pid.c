/**
The Process Identification Stuff


**/

#include <pid.h>
#include <limits.h>
#include <queue.h>
#include <synch.h>




// this will be incremented until it reaches the limit defined in os161/kern/include/kern/limits.h
// set it to limit -1 because we will increment and return the value
int PID_counter = __PID_MIN - 1;

struct queue *PID_queue;
struct lock *l;




// returns a new process id or -1 if no more process ids
int get_new_process_id(){

    
    int new_id = -1;

    lock_acquire(l);


    // see if we can recycle an ID (we do this first because otherwise we will get a huge queue of old IDs)
    if (!queue_isempty(PID_queue)){
        new_id = (int) queue_front(PID_queue);
        // remove that item from the queue
        queue_pop(PID_queue);

    }else{

        // otherwise, see if we can increment the PID counter
        if(PID_counter < __PID_MAX){
            // we can increment
            PID_counter++;

            // return that new value
            new_id = PID_counter;
        } // else, we will just return new_id which should be -1
    }


    lock_release(l);

    return new_id;
};


// releases a used process id to be reused
void release_process_id(int i){

    lock_acquire(l);

    queue_push(PID_queue, (void*) i);

    lock_release(l);

};



// initializes the PID system 
<<<<<<< HEAD
void pid_bootstrap(){
=======
void bootstrap_pid(){
>>>>>>> 457c67a28cb19cd755dd18b2ed289d4f7fd1919b

    // intialize the queue
    PID_queue = queue_create();

    // intialize the lock
    l = lock_create("pid_lock");
};








