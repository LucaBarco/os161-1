#include <types.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <lib.h>
#include <mips/trapframe.h>
#include <thread.h>
#include <proc.h>
#include <current.h>
#include <syscall.h>


// function to compare pids
static
int
proc_comparator(void* left, void* right)
{
    struct proc *l = (struct proc*)left;
    int r = *(int*)right;
    return l->PID - r;
}


/*
--- exit
Does process has child processes? (!empty(child_process_list))
if yes:
destroy all semaphores from the childs process threads (we don’t need them anymore)
set variable parent from all its childs structs to NULL
Clean up process ressources (free allocated space and pid) of all its child processes that are waiting that parent “joins”. 
Here we can also use the thread join mechanism again. 
The system call triggers the kernel to change the state of the waiting child process threads such that the cpu can cleanup the ressources.

Check if process has parent (parent != NULL)
if yes:
store exitcode in process struct (returnvalue = exitcode)
increment thread semaphore to maybe wake up the parent
process structure remains until parent “joins” or until it gets cleaned up
if no:
Clean up process ressources (free allocated space and pid)
*/
void sys_exit(int exitcode) {

	struct thread* curt = curthread;
	struct proc* curp = curt->t_proc;
	struct proc* childp = NULL;
	struct thread* childt = NULL;

	// lock process struct
	//spinlock_acquire(&childp->p_lock);

	// remove join property of child processes
	lock_acquire(&curp->p_childlist_lock);
	if(list_isempty(&curp->p_childlist) == 0){
		while(list_isempty(&curp->p_childlist) == 0)
		{
			childp = (struct proc*)list_front(&curp->p_childlist);
			if(childp == NULL)				// TODO make shure that this not happens
			{
				//spinlock_release(&childp->p_lock);
				lock_release(&curp->p_childlist_lock);
				// TODO die
			}

			childp = (struct proc*)list_remove(&curp->p_childlist, (void*) childp, &proc_comparator);
			//spinlock_acquire(&childp->p_lock);


			childt = threadarray_get(&childp->p_threads, 0);
			if( childt == NULL)				// TODO make shure that this not happens
			{
				//spinlock_release(&childp->p_lock);
				lock_release(&curp->p_childlist_lock);
				// TODO die
			}

			// update thread variables of child process
			childt->has_parent = false;
			childt->t_parent = NULL;
			sem_destroy(childt->t_join_sem_child);
			sem_destroy(childt->t_join_sem_parent);

			//spinlock_release(&childp->p_lock);
			curt->t_childs_to_join--;
		}
		
	}
	lock_release(&curp->p_childlist_lock);

	curp->p_returnvalue = exitcode;


	// TODO cleanup process ressouces

	// unlock process struct
	//spinlock_release(&childp->p_lock);

	// exit thread should cleanup the thread and release waiting parent processes
	//thread_exit(exitcode);
}


