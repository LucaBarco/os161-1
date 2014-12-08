#include <diskmap.h>
#include <types.h>
#include <lib.h>
#include <vm.h>
#include <spinlock.h>
#include <bitmap.h>
#include <coremap.h>
#include <vnode.h>
#include <vfs.h>
#include <uio.h>
#include <kern/fcntl.h>
#include <kern/errno.h>
#include <kern/stat.h>


// allocate a static spinlock
static struct spinlock diskmap_lock;

struct bitmap* diskmap;

struct vnode* swap_disk;




unsigned int free_dm_pages;
unsigned int number_of_disk_pages;


// acquires the diskmap lock
void dm_acquire_lock(void){
    spinlock_acquire(&diskmap_lock);
}

// releases the diskmap lock
void dm_release_lock(void){
    spinlock_release(&diskmap_lock);
}

// return number of pages
unsigned int dm_get_size(void) {
    return diskmap->nbits;
}

// returns true if the page on disk is not occupied, false otherwise
bool dm_is_free(unsigned int page_index){ 
    KASSERT(page_index < diskmap->nbits);

    int ret = bitmap_isset(diskmap, page_index);
    return ret == 0? true : false;
}

unsigned int dm_get_number_of_free_pages(){
    return free_dm_pages;
}



// sets the specified page to free
void dm_set_free(unsigned int page_index) {
    KASSERT(page_index < diskmap->nbits);


    if(dm_is_free(page_index)) {
        // page is already free. // TDOD: is this an error?
        KASSERT(false);
    }

    free_dm_pages++;
    KASSERT(free_dm_pages <= number_of_disk_pages);

    bitmap_unmark(diskmap, page_index);
}


// sets the specified page to occupied
void dm_set_occupied(unsigned int page_index){ 
    KASSERT(page_index < diskmap->nbits);
    
    if(!dm_is_free(page_index)) {
        // page is already in use
        KASSERT(false);
    }

    KASSERT(free_dm_pages != 0);

    free_dm_pages--;
    

    bitmap_mark(diskmap, page_index);
}


// get the index of a free page. page will be marked as occupied. returns false if disk full
bool dm_get_free_page(unsigned int* page_index){
    KASSERT(page_index != NULL);

    for(unsigned int i = 0; i < diskmap->nbits; i++){
        if(dm_is_free(i)){
            *page_index = i;

            dm_set_occupied(i);

            return true;
        }
    }

    *page_index = 0;
    return false;
}




// sets up the space in virtual memory to hold the diskmap 
// !!! We call alloc_kpages several times to get contiguous memory.
// !!! Since bootstrap is called directly after the coremap bootstrap we assume we get contiguous memory
void diskmap_bootstrap(void){

    // get the number of available pages
    unsigned int number_of_pages_avail = get_coremap_size();

    // number of available pages upscaled
    number_of_disk_pages = number_of_pages_avail * PHYS_MEM_SCALE;

    free_dm_pages = number_of_disk_pages;

    // calculate number of pages needed to store the diskmap (bits and bitmap struct)
    unsigned int number_of_pages = DIVROUNDUP(DIVROUNDUP(number_of_disk_pages, 8) + (unsigned int)sizeof(struct bitmap), PAGE_SIZE); 

    // 512 MB memory max
    // 4096 Byte a page
    // -> 131072 pages
    // pages needed for swapping = bits needed for diskmap
    // 131072 pages * 16 = 2097152 bits
    //                   = 262144 Byte
    //                   = 256 k Byte
    //                                  -> 64 pages max

    // we assume that we can allocate gontiguous pages since we call this function right after coremap_bootstrap()
    // allocate at least one page for the bitmap
    diskmap = (struct bitmap*) alloc_kpages(1);
    diskmap->v = (WORD_TYPE *)(diskmap + sizeof(struct bitmap));
    KASSERT(diskmap != NULL);
    for(unsigned int i = 1; i < number_of_pages; i++) {
        vaddr_t ret = alloc_kpages(1);
        KASSERT(ret != (vaddr_t)NULL);
    }
    
    (void) number_of_pages;

    // create bitmap
    bitmap_create_diskmap(diskmap, number_of_disk_pages);

    // do selftest
    diskmap_selftest();

    // finally intialize the spinlock
    spinlock_init(&diskmap_lock);
}

int swap_bootstrap(){
    char filename[] = "lhd0raw:";

    // try to open the vnode
    int res = vfs_open(filename, O_RDWR, 0, &swap_disk); //VFS open _will_ mangle with the filename char
    
    KASSERT(res == 0);
    struct stat s;
    VOP_STAT(swap_disk, &s);
    KASSERT((((off_t)dm_get_size()) << 12) < s.st_size);
    return res;
    

}

//reads a page out from disk onto physical memory
int read_page(unsigned int page_index, vaddr_t kpage_addr) {
    struct iovec iov;
    struct uio io;
    uio_kinit(&iov,&io,(void*)kpage_addr,PAGE_SIZE,((off_t)page_index) << 12, UIO_READ);
    int res = VOP_READ(swap_disk, &io);
    return res;
}

//writes a page from physical memory out to disk, returns the disk page index
int write_page(vaddr_t kpage_addr, unsigned int * ret) {
    dm_acquire_lock();
    
    unsigned int page_index = 0xFFFFFFFF;
    if(!dm_get_free_page(&page_index)){
        dm_release_lock();
        return ENOMEM;
    }
        

    dm_release_lock();

    struct iovec iov;
    struct uio io;
    uio_kinit(&iov,&io,(void*)kpage_addr,PAGE_SIZE,((off_t)page_index) << 12, UIO_WRITE);
    int res = VOP_WRITE(swap_disk, &io);
    *ret = page_index;
    return res;
}




// performs a selftest on the diskmap
void diskmap_selftest(void){

    dm_acquire_lock();

    KASSERT(diskmap != NULL);
    KASSERT(diskmap->nbits > 0);
    KASSERT(diskmap->v != NULL);

    // check the first bits; have to be free
    KASSERT(dm_is_free(0));
    KASSERT(dm_is_free(1));
    KASSERT(dm_is_free(2));

    // check occupy specific bit
    unsigned int bit = 23;
    KASSERT(dm_is_free(bit));
    dm_set_occupied(bit);
    KASSERT(!dm_is_free(bit));
    dm_set_free(bit);
    KASSERT(dm_is_free(bit));

    // check get free bit
    dm_get_free_page(&bit);
    KASSERT(!dm_is_free(bit));
    dm_set_free(bit);
    KASSERT(dm_is_free(bit));


    KASSERT(number_of_disk_pages == free_dm_pages);
    dm_release_lock();
}

