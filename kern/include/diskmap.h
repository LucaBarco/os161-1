#ifndef _H_DISKMAP_
#define _H_DISKMAP_

#include <types.h>


// acquires / releases the diskmapmap lock
void dm_acquire_lock(void);
void dm_release_lock(void);

// returns true if the page on disk is not occupied, false otherwise
bool dm_is_free(unsigned long page_index);

// sets the specified page to free
void dm_set_free(unsigned long page_index);

// sets the specified page to occupied
void dm_set_occupied(unsigned long page_index);

// get the index of a free page. page will be marked as occupied. returns false if disk full
bool dm_get_free_page(unsigned long* page_index);

// sets up the space in virtual memory to hold the diskmap 
// !!! We call alloc_kpages several times to get contiguous memory.
// !!! Since bootstrap is called directly after the coremap bootstrap we assume we get contiguous memory
void diskmap_bootstrap(void);


#endif // _H_DISKMAP_

