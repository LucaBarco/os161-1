/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <lib.h>
#include <mips/trapframe.h>
#include <thread.h>
#include <current.h>
#include <syscall.h>
#include <limits.h>


/*
 * System call dispatcher.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. 64-bit arguments are passed in *aligned*
 * pairs of registers, that is, either a0/a1 or a2/a3. This means that
 * if the first argument is 32-bit and the second is 64-bit, a1 is
 * unused.
 *
 * This much is the same as the calling conventions for ordinary
 * function calls. In addition, the system call number is passed in
 * the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, or v0 and v1 if 64-bit. This is also like an ordinary
 * function call, and additionally the a3 register is also set to 0 to
 * indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/user/lib/libc/arch/mips/syscalls-mips.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * If you run out of registers (which happens quickly with 64-bit
 * values) further arguments must be fetched from the user-level
 * stack, starting at sp+16 to skip over the slots for the
 * registerized values, with copyin().
 */
void
syscall(struct trapframe *tf)
{
	int callno;
	int32_t retval;
	int err;

	KASSERT(curthread != NULL);
	KASSERT(curthread->t_curspl == 0);
	KASSERT(curthread->t_iplhigh_count == 0);

	callno = tf->tf_v0;

	/*
	 * Initialize retval to 0. Many of the system calls don't
	 * really return a value, just 0 for success and -1 on
	 * error. Since retval is the value returned on success,
	 * initialize it to 0 by default; thus it's not necessary to
	 * deal with it except for calls that return other values,
	 * like write.
	 */

	retval = 0;

	switch (callno) {
	    case SYS_reboot:
		err = sys_reboot(tf->tf_a0);
		break;

	    case SYS___time:
		err = sys___time((userptr_t)tf->tf_a0,
				 (userptr_t)tf->tf_a1);
		break;

	    /* Add stuff here */

	    default:
		kprintf("Unknown syscall %d\n", callno);
		err = ENOSYS;
		break;
	}


	if (err) {
		/*
		 * Return the error code. This gets converted at
		 * userlevel to a return value of -1 and the error
		 * code in errno.
		 */
		tf->tf_v0 = err;
		tf->tf_a3 = 1;      /* signal an error */
	}
	else {
		/* Success. */
		tf->tf_v0 = retval;
		tf->tf_a3 = 0;      /* signal no error */
	}

	/*
	 * Now, advance the program counter, to avoid restarting
	 * the syscall over and over again.
	 */

	tf->tf_epc += 4;

	/* Make sure the syscall code didn't forget to lower spl */
	KASSERT(curthread->t_curspl == 0);
	/* ...or leak any spinlocks */
	KASSERT(curthread->t_iplhigh_count == 0);
}

/*
 * Enter user mode for a newly forked process.
 *
 * This function is provided as a reminder. You need to write
 * both it and the code that calls it.
 *
 * Thus, you can trash it and do things another way if you prefer.
 */
void
enter_forked_process(struct trapframe *tf)
{
	(void)tf;
}


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

Create new address space and copy the content from parent address space to child address space (as_copy(parent_as, child_as) ) (this should copy the memory content from the parent to the child space)
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
int
fork (void){
	int pid = 0;
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

	// allocate pid
	
	// check if generated pid is valid
	if(pid < PID_MIN || pid > PID_MAX)
	{
		return -1; // TODO generate error code for this case
	}




	return 0;
}

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
int
waitpid(int pid, int *status, int options) {
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

	return 0;
}

/*
--- exit
Does process has child processes? (!empty(child_process_list))
if yes:
destroy all semaphores from the childs process threads (we don’t need them anymore)
set variable parent from all its childs structs to NULL
Clean up process ressources (free allocated space and pid) of all its child processes that are waiting that parent “joins”. Here we can also use the thread join mechanism again. The system call triggers the kernel to change the state of the waiting child process threads such that the cpu can cleanup the ressources.
Check if process has parent (parent != NULL)
if yes:
store exitcode in process struct (returnvalue = exitcode)
increment thread semaphore to maybe wake up the parent
process structure remains until parent “joins” or until it gets cleaned up
if no:
Clean up process ressources (free allocated space and pid)
*/
void _exit(int exitcode) {
	return;
}




