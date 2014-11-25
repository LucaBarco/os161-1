#include <coremap.h>
#include <types.h>
#include <lib.h>
#include <vm.h>


// the borders of the ram after ram_bootstrap
paddr_t firstpaddr; // first address
paddr_t lastpaddr; // last address


struct cm_entry* coremap;


// sets up the space in virtual memory to hold the coremap
void coremap_bootstrap(void){

    // get the physical space which is available, we can not use steal mem at this point anymore
    ram_getsize(&firstpaddr, &lastpaddr);

    // get the number of bytes which will be available
    unsigned int number_of_bytes_avail = lastpaddr - firstpaddr; // I think that has to be +1.. TODO not critical

    // get the number of available pages
    unsigned int number_of_pages_avail = number_of_bytes_avail / PAGE_SIZE;

    /* testing struct sizes */
    size_t struct_size = sizeof(struct cm_entry);

    size_t struct_size2 = sizeof(struct empty_struct);

    size_t struct_size3 = sizeof(unsigned int);

    int page_size = PAGE_SIZE;
    (void) page_size;

    (void) struct_size2;

    (void) struct_size3;


    /* end testing struct sizes */

    // calculate the size of the coremap (in bytes)
    unsigned int coremap_size = struct_size * number_of_pages_avail;

    // calculate how much pages that would be, round up to be page aligned
    unsigned int number_of_pages =  DIVROUNDUP( coremap_size , PAGE_SIZE ); // TODO out of some reason that does not work

    // this seems hacky and probably is hacky TODO
    //number_of_pages = max(number_of_pages, 1u);


    KASSERT(number_of_pages > 0);


    // Print that out
    kprintf("%u pages (%u bytest) occupied for the coremap\n", number_of_pages, coremap_size);   



    struct dummy_table_entry test;

    // get a pointer for the coremap. we have to translate the physical address into 
    coremap = (struct cm_entry*) PADDR_TO_KVADDR(firstpaddr);

    // initialize the coremap
    for(unsigned int i = 0; i < number_of_pages_avail; i++){
        coremap[i].free = 1;
        coremap[i].kernel = 0;
        coremap[i].page_table_entry = &test;
    }

    // now try to steal that space for the coremap
    // we do this by incrementing the firstpaddr pointer
    firstpaddr += number_of_pages * PAGE_SIZE;






}

// returns true if the page in physical memory is not occupied, false otherwise
bool is_free(unsigned int page){ 

    (void) page; 

    return false;
}

// sets the specified page to occupied
void set_occupied(unsigned int page){ 

    (void) page; 

}

// sets the specified page to free and writes deadbeef
void set_free(unsigned int page){ 
    (void) page; 
}

// checks if the speciefied page is a kernel page
bool is_kernel_page(unsigned int page){ 
    (void) page; 

    return 0;
}

// sets the specified page as a kernel page
void set_kernel_page(unsigned int page){ 
    (void) page; 
}

// sets the specified page as a user page
void set_user_page(unsigned int page){ 
    (void) page; 

}




