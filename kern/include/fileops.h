#ifndef _FILE_OPS_H_
#define _FILE_OPS_H_


#include <types.h>
#include <addrspace.h>
#include <vnode.h>
#include <synch.h>




// define the file descriptor struct
struct file_descriptor{
	struct vnode *fd_file;
	int mode;
	int offset;
	struct lock *fd_lock;
	char *filename;
};


// creates a file descriptor. return null if not successfull
struct file_descriptor* fd_create(void);


// destroys a given file descriptor
void fd_destroy(struct file_descriptor* fd);







#endif
