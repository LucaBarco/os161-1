//#include <kern/errno.h>
//#include <kern/syscall.h>
//#include <types.h>
//#include <machine/trapframe.h>
//#include <pid.h>

//#include <thread.h>
//#include <proc.h>
//#include <current.h>
//#include <addrspace.h>
//#include <syscall.h>
//#include <vnode.h>

//#include <kern/errno.h>
//#include <kern/syscall.h>
//#include <lib.h>
//#include <limits.h>


#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <limits.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <pid.h>
#include <syscall.h>
#include <mips/trapframe.h>



/*
 * Enter user mode for a newly forked process.
 *
 * This function is provided as a reminder. You need to write
 * both it and the code that calls it.
 *
 * Thus, you can trash it and do things another way if you prefer.
 */
static int enter_forked_process(void *tf,  unsigned long n)
{
	(void)n;
	
	// copy trapframe
	struct trapframe trapf = (*((struct trapframe *) tf));// = *((struct trapframe*)tf);
	//trapf = kmalloc(sizeof(struct trapframe));
	//memcpy(trapf, tf, sizeof(struct trapframe));

	// Set the trapframe values
	// set returnvalue 0
	trapf.tf_v0 = 0;
	// signal no error
	trapf.tf_a3 = 0;
	// increase pc such that systemcall is not called again
	trapf.tf_epc += 4;

	// free allocated memory
	kfree(tf);

	// change back to usermode
	mips_usermode(&trapf);

	// Panic if user mode returns // should not happen
	panic("Returned from user mode!");
	return -1;
}


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

int sys_fork(struct trapframe *tf, int32_t *ret){
	int new_pid = 0;
	struct addrspace *new_as = NULL;
	struct proc* new_proc = NULL;
	struct thread* new_thread = NULL;
	struct thread* curt = curthread;
	struct proc* curp = curt->t_proc;
	struct trapframe *trapf;
	char name[16];
	int result;
	spinlock_acquire(&curp->p_lock);

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
      	result = snprintf(name, sizeof(name), "child_%d", new_pid);
	if (result < 0 ) {
		return result; 
	}

	// create child process
	new_proc = proc_create_runprogram(name);
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
	
	memcpy(ret, &new_pid, sizeof(int));

	/* Create a new address space and copy content of current process in it. */
	result = as_copy(curp->p_addrspace, &new_as);
	if (result || new_as == NULL) {
		proc_destroy(new_proc);
		return result; 
	}
	new_proc->p_addrspace = new_as;

	// Copy the trapframe to the heap so it's available to the child
	trapf = kmalloc(sizeof(*tf));
	memcpy(tf,trapf,sizeof(*tf));

	result = thread_fork(name, &new_thread, new_proc, &enter_forked_process, trapf, 0);
	if (result) {
		kfree(trapf);
		proc_destroy(new_proc);
		return result; 
	}


	/* we don't need to lock proc->p_lock as we have the only reference */
	if (new_proc->p_cwd != NULL) {
		VOP_INCREF(curp->p_cwd);
		new_proc->p_cwd = curp->p_cwd;
	}
	

	/* Thread subsystem fields */
	list_push_back(&curp->p_childlist, (void*)new_proc);
	spinlock_release(&curp->p_lock);
	
	return 0;
}
