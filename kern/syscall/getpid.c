#include <types.h>
#include <clock.h>
#include <copyinout.h>
#include <current.h>
#include <syscall.h>



int sys_getpid(){


    // just return the current thread's process' pid
    int cur_pid = curthread->proc.pid;

    int result = cur_pid;

    return result;

}