
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <mips/tlb.h>
#include <spl.h>
#include <vm.h>
#include <coremap.h>


void
vm_bootstrap(void)
{
	coremap_bootstrap();
}



/* Allocate/free some kernel-space virtual pages */
vaddr_t
alloc_kpages(int npages)
{
	acquire_cm_lock();

	vaddr_t page_addr = 0;
	unsigned int  page_index;
	bool ret = false;

	// for simplicity we only allow to allocate one pace per call
	KASSERT(npages == 1);

	// get a free page
	ret = get_free_page(&page_index);
	// return null if no page is available or strange page_index received
	if (ret == false || page_index <= 0) {		
		release_cm_lock();
		KASSERT(false);
		// return NULL;		// TODO: return null instead of kassert?
	}

 	// occupy page
    set_occupied(page_index);

    // Additional test
    /*
    // check if occupying worked
    ret = is_free(page_index);
	if (ret) {		
		release_cm_lock();
		KASSERT(false);
		// return NULL;		// TODO: return null instead of kassert?
	}
	*/

	// Set kernel flag
    set_kernel_page(page_index);

    // Additional tests
    /*
    // check if kernel flag is set
    ret = is_kernel_page(page_index);
	if (ret == false) {		
		release_cm_lock();
		KASSERT(false);
		// return NULL;		// TODO: return null instead of kassert?
	}
	*/

	// get address of page get_page_vaddr returns KVADDR
	page_addr = get_page_vaddr(page_index);

	release_cm_lock();

	return page_addr;
}

void
free_kpages(vaddr_t addr)
{
	acquire_cm_lock();
	int page_index = 0;
	bool ret = false;

	// check if vaddr_t is in kernel area
	if(addr < MIPS_KSEG0 || MIPS_KSEG1 <= addr) {
		release_cm_lock();
		KASSERT(false);
	}

	// translate KVADDR to VADDR -> happens in get_page_index()

	// get page index
	page_index = get_page_index(addr);


	// page should be in use
    ret = is_free(page_index);
	if (ret == false) {		
		release_cm_lock();
		KASSERT(false);
	}

	// page should not be marked as kernel page
    ret = is_kernel_page(page_index);
	if (ret) {		
		release_cm_lock();
		KASSERT(false);
	}

	// free page and deadbeef it
	set_free(page_index);


	release_cm_lock();
}


void
vm_tlbshootdown_all(void)
{
    int spl = splhigh();
    
    int i;
	for (i=0; i<NUM_TLB; i++) {
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	vm_tlbshootdown_all();
}
