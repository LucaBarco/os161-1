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
#include <uio.h>

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


	fd->refcount = 0;


	// the rest (mode, offset, and filename) will be set on call of fd_open()
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


// creates a file descriptor, will return approriate error code or 0 on success
int fd_open(struct fd_table* fdt, char* filename, int flags, int* fd_index){

	// TODO, check if EFAULT	filename was an invalid pointer.


	// I think we don't have to do this here since we will only have one thread but you never know..
	//lock_acquire(fdt->lock);
	

	// generate a vnode pointer
	struct vnode* mvnode;

	// copy the filename since vfs_open will fiddle around with it
	char* filename_copy =  NULL;
	filename_copy = kstrdup(filename);

	// try to open the vnode
	int res = vfs_open(filename, flags, 0, &mvnode);

	// see if this worked
	if(res){
		return res;
	}

	// create the file descriptor
	struct file_descriptor* fd_temp = add_file_descriptor(fdt, filename_copy, flags);

	// return its id
	*fd_index = fd_temp->index;
	

	// free the filename copy
	kfree(filename_copy);

	if(fd_temp==NULL){ // probably too many open files.
		vfs_close(mvnode);
		return EMFILE;
	}

	// add the vnode to the file descriptor
	fd_temp->vnode = mvnode;

	// add one reference
	fd_temp->refcount++;


	//lock_release(fdt->lock);
	return 0;
}


                                       
int fd_read(struct file_descriptor* fd, char *kbuf, size_t buflen, size_t* read_bytes){

	

	int res = 0;

	// lock
	lock_acquire(fd->fd_lock);

	

	struct iovec iov;
	struct uio io;

	
	off_t old_offset = fd->offset;

	// initialize iovec and uio for reading
	uio_kinit(&iov, &io, kbuf, buflen, fd->offset, UIO_READ); // this sets it to UIO_SYSSPACE, does it need to be UIO_USERSPACE? TODO


	// READ
	res = VOP_READ(fd->vnode, &io);

	
	// update offset in file descriptor
	fd->offset = io.uio_offset; // TODO CHECK ON FAILURE

	// set read bytes
	*read_bytes = (size_t) ((unsigned int) io.uio_offset) - ((unsigned int) old_offset );

	// free the stuff
	//kfree(&iov);
	//kfree(&io);

	lock_release(fd->fd_lock);
	return 0;
};



int fd_write(struct file_descriptor* fd, char* kbuf, size_t buflen, size_t* written_bytes){

	

	int res = 0;

	// lock
	lock_acquire(fd->fd_lock);

	

	struct iovec iov;
	struct uio io;

	off_t old_offset = fd->offset;
	
	// initialize iovec and uio for reading
	uio_kinit(&iov, &io, kbuf, buflen, fd->offset, UIO_WRITE); // this sets it to UIO_SYSSPACE, does it need to be UIO_USERSPACE? TODO


	// READ
	res = VOP_WRITE(fd->vnode, &io);

	
	// update offset in file descriptor
	fd->offset = io.uio_offset; // TODO CHECK ON FAILURE

	// set written bytes
	*written_bytes = (size_t) ((unsigned int) io.uio_offset) - ((unsigned int) old_offset );


	// free the stuff
	//kfree(&iov);
	//kfree(&io);

	lock_release(fd->fd_lock);
	return 0;
};


int fd_close(struct fd_table* fdt, struct file_descriptor* fd){

	(void) fdt;
	(void) fd;

	// decrease refcount
	fd->refcount--;

	// if refcount is 0 than it is save to close the vnode
	if(fd->refcount ==0){
		vfs_close(fd->vnode);
	}

	// TODO delete from file descriptor table



	return 0;


}

struct file_descriptor* get_fd(struct fd_table* fdt, int fd_id){

	return fdt->fds[fd_id];

	//return NULL;
}



