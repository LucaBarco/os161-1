#ifndef _FILE_OPS_H_
#define _FILE_OPS_H_


#include <types.h>
#include <addrspace.h>
#include <vnode.h>
#include <hashtable.h>
#include <list.h>
#include <synch.h>
#include <proc.h>
#include <limits.h>





// define the file descriptor struct
struct file_descriptor{
	char *filename;

	struct vnode *vnode;

	// O_RDONLY, etc...
	int flags;

	// the current offset of this filenode, this has to be set in the VNODE prior doing anything
	int offset;


	int index;

	struct lock *fd_lock;

};


// creates a file descriptor. return null if not successfull
struct file_descriptor* fd_create(void);


// destroys a given file descriptor
void fd_destroy(struct file_descriptor* fd);



//create the file table struct
struct fd_table{

	// the process of the file_table
	struct proc* proc;

	// the hashtable which points from int -> file_descriptor
	struct hashtable* ht;

	// dummy impl with indexes
	struct file_descriptor* fds[__OPEN_MAX];
	
	// the lock
	struct lock* lock;

	// a list of all open file descriptors - we need this on copy
	struct list* list;

	// the counter for file_descriptors
	int counter;

	// a queue of free file_descriptors TODO

};

// creates a new file descriptor table for the given process - does not attach the table to the process
struct fd_table* fd_table_create(struct proc*);

void fd_table_destroy(struct fd_table* fdt);

// get a new fd index, returns -1 if full
int get_new_fd_index(struct fd_table* fdt);

// add a file descriptor to the table
struct file_descriptor* add_file_descriptor(struct fd_table* fdt, char* filename, int flags);




/* open, read, write, close starts here. boom. */

// creates a file descriptor for reading, will return approriate error code or 0 on success
int fd_open(struct fd_table* fdt, char* filename, int flags, struct file_descriptor* fd);











#endif
