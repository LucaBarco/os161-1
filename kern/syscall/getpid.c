#include <types.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <lib.h>
#include <mips/trapframe.h>
#include <thread.h>
#include <proc.h>
#include <current.h>
#include <syscall.h>



int sys_getpid(userptr_t pid){


    // just return the current thread's process' pid
    int cur_pid = curthread->proc.pid;

    int result = copyout(&cur_pid, pid, sizeof(int));

    if(result){
        return result;
    }

    return 0;

}