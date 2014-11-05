#include <types.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <lib.h>
#include <mips/trapframe.h>
#include <thread.h>
#include <proc.h>
#include <current.h>
#include <syscall.h>



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
void sys_exit(int exitcode) {
	return;
}


