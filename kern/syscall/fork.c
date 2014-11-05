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
x	From the syscall() function we get the parent trap frame as argument.
x	We try to do as much tests as possible at the start of fork() so that we can fail early if necessary.
x	Check if there are already too many processes on the system
x	if yes -> error: ENPROC & return -1
x	Check if current user already has too many processes
x	if yes -> error: EMPROC & return -1
x	Allocate new pid (getpid()) and assign it to new child process

Create trapframe for child and copy all values of the parret trapframe

x	Create new address space and copy the content from parent address space to child address space (as_copy(parent_as, child_as) ) (this should copy the memory content from the parent to the child space)
x	Create a new process skeleton (proc_create_runprogram(name)), whereas name is given by the parent name + “_child”.
x	Create child process thread
x	Add parent information to the child process thread (initialises the semaphore for the waitpid mechanism)
Create file descriptor table for child process and copy content of parent file descriptor table into it.
Create open file table for child and copy content of parent open file handle table into it. Remove files from the child open file table that are opened as writable in the parent list. 
x	Check if sufficient virtual memory for the new process was available (that means check if all create operations succeed)
x	if not -> error: ENOMEM + return -1 + cleanup
Add child process to child_process_list of parent process
x	Set parent variable in child to current parent
Modify return value of child trapframe to 0 (trapframe->v0 = 0)
Modify return value of parent to the child pid (trapframe->v0 = child pid)
return (if no error occured)
*/

int sys_fork(void){
	int new_pid = 0;
	struct addrspace *new_as = NULL;
	struct proc* new_proc = NULL;
	struct thread* new_thread = NULL;
	struct thread* curt = curthread;
	struct proc curp = curt->t_proc;
	char name[16];
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

	// generate process name
      	result = sprintf(name, "child_%d", new_pid); 
	if (result < 0 ) {
		return result; 
	}

	// create child process
	new_proc = proc_create_runprogram(name)
	if (new_proc == NULL) {
		return -1; 
	}

	new_pid = new_proc->PID;
	// check if generated pid is valid
	/*
	if(pid < PID_MIN || pid > PID_MAX)
	{
		proc_destroy(proc);
		return EPIDOOR; 
	}
	*/
	
	/* Create a new address space and copy content of current process in it. */
	result = as_copy(curp->p_addrspace, &new_as);
	if (result || new_as == NULL) {
		proc_destroy(new_proc);
		return result; 
	}
	new_proc->p_addrspace = new_as;

	// create new thread
	new_thread = thread_create(name);
	if (new_thread == NULL) {
		proc_destroy(new_proc);
		//return -1;
		return ENOMEM;
	}	

	// attach new thread to new process
	result = proc_addthread(new_proc, new_thread);
	if (result) {
		thread_destroy(new_thread)
		proc_destroy(new_proc);
		return -1;
	}

	/* Allocate a stack */
	newthread->t_stack = kmalloc(STACK_SIZE);
	if (newthread->t_stack == NULL) {
		proc_remthread(new_thread);
		thread_destroy(new_thread);
		proc_destroy(new_proc);
		return ENOMEM;
	}
	thread_checkstack_init(new_thread);
	// TODO copz stack

	// increase number of childs to join
	curt->t_childs_to_join++;

	// create semaphore
	new_thread->t_join_sem_parent = sem_create(name, 0);

	// destroy thread if create semaphore did not work
	if (new_thread->t_join_sem_parent == NULL) {
		proc_remthread(new_thread);
		thread_destroy(new_thread);
		proc_destroy(new_proc);
		return -1;
	}


	new_thread->t_join_sem_child = sem_create(name, 0);

	// destroy thread if create semaphore did not work
	if (new_thread->t_join_sem_child == NULL) {
		proc_remthread(new_thread);
		thread_destroy(new_thread);
		proc_destroy(new_proc);
		sem_destroy(new_thread->t_join_sem_parent);
		return -1;
	}
	


	/* Process subsystem fields */
	new_proc->p_parent = curp;

	spinlock_acquire(&curp->p_lock);
	/* we don't need to lock proc->p_lock as we have the only reference */
	if (new_proc->p_cwd != NULL) {
		VOP_INCREF(curp->p_cwd);
		new_proc->p_cwd = curp->p_cwd;
	}
	spinlock_release(&curp->p_lock);


	/* Thread subsystem fields */
	proclist_addtail(curp->p_childlist, new_proc);
	new_thread->t_in_interrupt = curt->t_in_interrupt;	/* Are we in an interrupt? */
	new_thread->t_curspl = curt->t_curspl;			/* Current spl*() state */
	new_thread->t_iplhigh_count = curt->t_iplhigh_count;
	// set parent thread
	new_thread->t_parent = curt;
	new_thread->has_parent = true;
	new_thread->t_cpu = curt->t_cpu;
	new_thread->t_proc = curt->t_proc;

	

//----------------  // TODO how to copy
	const char *t_wchan_name;	/* Name of wait channel, if sleeping */
	threadstate_t t_state;		/* State this thread is in */

	/*
	 * Thread subsystem internal fields.
	 */
	struct thread_machdep t_machdep; /* Any machine-dependent goo */
	struct threadlistnode t_listnode; /* Link for run/sleep/zombie lists */
	void *t_stack;			/* Kernel-level stack */
	struct switchframe *t_context;	/* Saved register context (on stack) */

//------------- // TODO how to copy

	/* Set up the switchframe so entrypoint() gets called */
	switchframe_init(new_thread, entrypoint, data1, data2);

	/* Lock the current cpu's run queue and make the new thread runnable */
	thread_make_runnable(new_thread, false);

	return 0;
//-----------


	
	return -1;
}
