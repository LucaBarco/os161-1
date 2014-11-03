#ifndef _PID_H_
#define _PID_H_


int PID_counter;
//struct queue *PID_queue;
//struct lock *l;


// returns a new process id or -1 if no more process ids
int get_new_process_id(void);

// releases a used process id to be reused
void release_process_id(int i);

// initializes the PID system 
<<<<<<< HEAD
void pid_bootstrap(void);
=======
void bootstrap_pid(void);
>>>>>>> 457c67a28cb19cd755dd18b2ed289d4f7fd1919b


#endif
