#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <limits.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <copyinout.h>
#include <syscall.h>

int execv(const char *program, char **args) {
    struct addrspace *new_as;
    struct addrspace *old_as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
    
    int i = 0;
    char ** argv = NULL;
    int argc = 0;
    //amount of memory left for arg strings
    int left = ARG_MAX;
    //length of a arg string
    int len = 0;
    //kernel location of string
    void * kstring;
    //new userspace location of string
    char * newstring;
    
	/* Open the file. */
	result = vfs_open((char*) program, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* Create a new address space. */
	new_as = as_create();
	if (new_as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(new_as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(new_as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}
    
    if(args != NULL) {
        // count argc
        proc_setas(old_as);
        while(true) {
            // make sure pointer is in userspace
            if((userptr_t)(args+(sizeof(void*)*argc+1)) > (userptr_t)USERSPACETOP)
                return EFAULT;
            
            if(args[argc] == NULL)
                break;
            
            argc++;
        }
        
        // create argv
        proc_setas(new_as);
        stackptr -= (vaddr_t)(argc*sizeof(char *));
        argv = (char **) stackptr;
        
        for(i = 0; i < argc; i++) {
            // switch back to old as
            proc_setas(old_as);
            
            // make sure string is valid and get length
            for(len = 0;; len++) {
                // make sure next char is in userspace
                if((userptr_t)(args[i]+(sizeof(char)*len+1)) > (userptr_t)USERSPACETOP) {
                    proc_setas(new_as);
                    as_deactivate();
                    proc_setas(old_as);
                    as_destroy(new_as);
                    return EFAULT;
                }
                
                if(args[i][len] == 0)
                    break;
            }
            left -= len;
            if(left < 0) {
                proc_setas(new_as);
                as_deactivate();
                proc_setas(old_as);
                as_destroy(new_as);
                return E2BIG;
            }
            
            // kmalloc for kernel string
            kstring = kmalloc(len);
            if(kstring == NULL) {
                proc_setas(new_as);
                as_deactivate();
                proc_setas(old_as);
                as_destroy(new_as);
                return ENOMEM;
            }
            
            //copyin the string
            result = copyin((userptr_t) args[i], kstring, len);
            if(result != 0) {
                kfree(kstring);
                proc_setas(new_as);
                as_deactivate();
                proc_setas(old_as);
                as_destroy(new_as);
                return result;
            }
            
            // switch back to new as
            proc_setas(new_as);
            
            //malloc for the string in the new address space
            stackptr -= (vaddr_t)len;
            newstring = (char*) stackptr;
            if(newstring == NULL) {
                kfree(kstring);
                as_deactivate();
                proc_setas(old_as);
                as_destroy(new_as);
                return ENOMEM;
            }
            
            // copyout, and kfree
            result = copyout(kstring, (userptr_t) newstring, len);
            kfree(kstring);
            if(result != 0) {
                as_deactivate();
                proc_setas(old_as);
                as_destroy(new_as);
                return result;
            }
            
            // put copyout location in argv
            argv[i] = newstring;
        }
    }

	/* Destroy the old address space. */
    proc_setas(old_as);
    as_deactivate();
    proc_setas(new_as);
	as_destroy(old_as);

	/* Warp to user mode. */
	enter_new_process(argc, (userptr_t) argv,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}
