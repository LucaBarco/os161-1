#include "fileops.h"

#include <types.h>
#include <addrspace.h>
#include <vnode.h>
#include <lib.h>
#include <hashtable.h>
#include <list.h>
#include <kern/fcntl.h>
#include <kern/errno.h>
#include <vfs.h>

// creates a new file descriptor
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
	return fd;
};


// destroys a file descriptor
void fd_destroy(struct file_descriptor* fd){

	// destroy the lock
	lock_destroy(fd->fd_lock);

	// free the memory
	kfree(fd);
};



// creates a new file descriptor table for the given process - does not attach the table to the process
struct fd_table* fd_table_create(struct proc* proc){

	struct fd_table *fdt;

	// init the strcut
	fdt = kmalloc(sizeof(struct fd_table));
	if(fdt == NULL){
		return NULL;
	}

	// create the lock
	fdt->lock = lock_create("file table lock");
	if(fdt->lock == NULL){
		kfree(fdt);
		return NULL;
	}

	// create the hashtable
	fdt->ht = hashtable_create();
	if(fdt->ht == NULL){
		lock_destroy(fdt->lock);
		kfree(fdt);
		return NULL;
	}

	// create the list
	fdt->list = list_create();
	if(fdt->list == NULL){
		lock_destroy(fdt->lock);
		hashtable_destroy(fdt->ht);
		kfree(fdt);
		return NULL;		
	}

	// set the process
	fdt->proc = proc;

	// set counter to -1
	fdt->counter = -1;

	// add the file desriptors 0 and 1, or so.. TODO!


	// the rest (mode, offset, and filename) will be set on call of open()
	return fdt;
};


void fd_table_destroy(struct fd_table* fdt){

	// destroy the lock
	lock_destroy(fdt->lock);

	// destroy the hashtable
	hashtable_destroy(fdt->ht);



	// destroy the list
	list_destroy(fdt->list);

	// free the memory
	kfree(fdt);
}


// get a new fd index, returns -1 if full
int get_new_fd_index(struct fd_table* fdt){

	fdt->counter++;
	return fdt->counter;

	// TODO: add queue and stuff

}


// add a file descriptor to the table
struct file_descriptor* add_file_descriptor(struct fd_table* fdt, char* filename, int flags){
	KASSERT(fdt != NULL);

	// get a new file descriptor index
	int new_index = get_new_fd_index(fdt);

	if(new_index == -1){
		return NULL;
	}

	// create the fd struct
	struct file_descriptor* fd;
	fd = fd_create();

	// set the file name
	fd->filename = kstrdup(filename);

	// set the flags
	fd->flags = flags;

	// set the new id
	fd->index = new_index;

	// add the file descritpor to the tables list and hashtable
	// TODO dummy, do this again with the hashtable
	fdt->fds[new_index] = fd;	

	return fd;
}


// creates a file descriptor for reading, will return approriate error code or 0 on success
int fd_open(struct fd_table* fdt, char* filename, int flags, struct file_descriptor* fd){

	// TODO, check if EFAULT	filename was an invalid pointer.


	// I think we don't have to do this here since we will only have one thread but you never know..
	lock_acquire(fdt->lock);
	

	// generate a vnode pointer
	struct vnode* mvnode;

	// try to open the vnode
	int res = vfs_open(filename, flags, 0, &mvnode);

	// see if this worked
	if(res){
		return res;
	}


	// create the file descriptor
	fd = add_file_descriptor(fdt, filename, flags);

	if(fd==NULL){ // probably too many open files.
		vfs_close(mvnode);
		return EMFILE;
	}

	// add the vnode to the file descriptor
	fd->vnode = mvnode;


	lock_release(fdt->lock);
	return 0;
}

