#include "fileops.h"

#include <types.h>
#include <addrspace.h>
#include <vnode.h>
#include <lib.h>





struct file_descriptor* fd_create(void){

	struct file_descriptor *fd;

	// init the strcut
	fd = kmalloc(sizeof(struct file_descriptor));
	if(fd == NULL){
		return NULL;
	}

	// create the lock
	fd->fd_lock = lock_create("fd lock");
	if(fd->fd_lock == NULL){
		kfree(fd);
		return NULL;
	}


	// the rest (mode, offset, and filename) will be set on call of open()

	return NULL;
};


void fd_destroy(struct file_descriptor* fd){

	(void) fd;

};


