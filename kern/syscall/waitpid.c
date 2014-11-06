#include <types.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <lib.h>
#include <mips/trapframe.h>
#include <proc.h>
#include <current.h>
#include <syscall.h>
#include <thread.h>

/*
--- waitpid
chek if pid argument named a nonexistent process
x	if yes -> error: ESRCH & return -1
Check if pid argument named a process that was not a child of the current process (check child_pid_list of parent)
x	if yes -> error: ECHILD & return -1
x	Check if status argument is an invalid pointer (status == NULL)
x	if yes -> error: EFAULT & return -1
x	Check if options argument requested invalid or unsupported options (options != 0)
x	if yes -> error: EINVAL & return -1

The parent wants to decrement the semaphore of the child process thread. The child process thread increments the semaphore if it exits. If the child has not finished (exit) yet, the parent will wait until the child exits. If the child already finished, parent don’t has to wait.
After a successfully wait (child process exits and parent process is up again and grabbed the return value of the child which is saved in the childs process struct)
Clean up child process ressources (free allocated space and pid)
Return child pid
*/
int sys_waitpid(int pid, int *status, int options) {
	// chek if pid argument named a nonexistent process
	if(false)// TODO
	{
		return ESRCH;
	}

	// check if pid argument named a process that was not a child of the current process
	if(false)// TODO
	{
		return ECHILD;
	}

	// check if status is invalid
	if(status == NULL)
	{
		return EFAULT;
	}

	// check if option argument is valid
	if( status != 0)
	{
		return EINVAL;
	}

	return -1;
}

