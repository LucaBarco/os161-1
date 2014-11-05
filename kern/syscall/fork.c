#include <types.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <lib.h>
#include <limits.h>
#include <mips/trapframe.h>
#include <thread.h>
#include <proc.h>
#include <current.h>
#include <syscall.h>
#include <pid.h>


/*
--- fork
From the syscall() function we get the parent trap frame as argument.
We try to do as much tests as possible at the start of fork() so that we can fail early if necessary.
Check if there are already too many processes on the system
x	if yes -> error: ENPROC & return -1
Check if current user already has too many processes
x	if yes -> error: EMPROC & return -1
Allocate new pid (getpid()) and assign it to new child process

Create trapframe for child and copy all values of the parret trapframe

x	Create new address space and copy the content from parent address space to child address space (as_copy(parent_as, child_as) ) (this should copy the memory content from the parent to the child space)
Create a new process skeleton (proc_create_runprogram(name)), whereas name is given by the parent name + “_child”.
 E.g. parent name = “parent” -> child name = “parent_child”
Create child process thread
Add parent information to the child process thread (initialises the semaphore for the waitpid mechanism)
Create file descriptor table for child process and copy content of parent file descriptor table into it.
Create open file table for child and copy content of parent open file handle table into it. Remove files from the child open file table that are opened as writable in the parent list. 
Check if sufficient virtual memory for the new process was available (that means check if all create operations succeed)
if not -> error: ENOMEM + return -1 + cleanup
Add child process to child_process_list of parent process
Set parent variable in child to current parent
Modify return value of child trapframe to 0 (trapframe->v0 = 0)
Modify return value of parent to the child pid (trapframe->v0 = child pid)
return (if no error occured)
*/

int sys_fork(void){
	int new_pid = 0;
	struct addrspace *new_as = NULL;
	struct thread* curt = curthread;
	struct proc curp = curt->t_proc;
	int result;
	

	// check if there are already too many processes on the system
	if(false)// TODO
	{
		return ENPROC;
	}

	// check if current user already has too many processes
	if(false)// TODO
	{
		return EMPROC;
	}

	new_pid = get_new_process_id();
	
	// check if generated pid is valid
	/*
	if(pid < PID_MIN || pid > PID_MAX)
	{
		return EPIDOOR; 
	}
	*/
	
	/* Create a new address space and copy content of current process in it. */

	result = as_copy(curp->p_addrspace, &curt);
	if (result) {
		return -1; // TODO check error message
	}
	if (new_as == NULL) {
		return -1; // TODO check error message
	}


	return -1;
}
