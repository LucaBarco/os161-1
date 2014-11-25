#ifndef _H_COREMAP_
#define _H_COREMAP_

#include <types.h>

struct dummy_table_entry{
	int test;
};

struct cm_entry{

    int free               				;  // indicates if the page is free
    int kernel             				;  // indicates if the page is a kernel page
    struct dummy_table_entry *page_table_entry;   	 // pointer to the page table entry
    //unsigned int                    : 30; // this is a huge waste of space. refactor at some point

};

struct empty_struct{};


// sets up the space in virtual memory to hold the coremap
void coremap_bootstrap(void);

// returns true if the page in physical memory is not occupied, false otherwise
bool is_free(unsigned int page);

// sets the specified page to occupied
void set_occupied(unsigned int page);

// sets the specified page to free and writes deadbeef
void set_free(unsigned int page);

// checks if the speciefied page is a kernel page
bool is_kernel_page(unsigned int page);

// sets the specified page as a kernel page
void set_kernel_page(unsigned int page);

// sets the specified page as a user page
void set_user_page(unsigned int page);





#endif

