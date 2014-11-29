
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
		return NULL;	
	}

 	// occupy page
    	set_occupied(page_index);

    	// Additional test
    	/*
    	// check if occupying worked
    	ret = is_free(page_index);
	if (ret) {		
		release_cm_lock();
		return NULL;
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
		return NULL;
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
/*
int
vm_fault(int faulttype, vaddr_t faultaddress)
{
    if (curproc == NULL) {
*/		/*
		 * No process. This is probably a kernel fault early
		 * in boot. Return EFAULT so as to panic instead of
		 * getting into an infinite faulting loop.
		 */
/*		return EFAULT;
	}

	as = proc_getas();
	if (as == NULL) {
*/		/*
		 * No address space set up. This is probably also a
		 * kernel fault early in boot.
fault		 */
/*		return EFAULT;
	}
	for(i=0;i<4;i++) 
	  {
	    
	    if(faultaddress >= as->segment_table[i].start && faultaddress < segment_table[i].end)
	      {
		if(!as->ignore_permissions) 
		  {
		    
		switch (faulttype) {
	    case VM_FAULT_READONLY:
	      if(!as->segment_table[i].write)
		{
	      
		//throw error
		}
	      break;
	    case VM_FAULT_READ:
	      
	      if(!as->segment_table[i].read)
		{
	      
		//throw error
		}
	    case VM_FAULT_WRITE:
	      if(!as->segment_table[i].write)
		{
	      
		//throw error
		}
		break;
	    default:
		return EINVAL;
	}
		  }
		
	      }
	    
	  }
	
	    if(!as->page_table[faultaddress >> 12].is_valid) 
	      {
		
		as->page-table[faultaddress >> 12].index = alloc_kpages(1);
		
	      }
	
     switch (faulttype) {
	    case VM_FAULT_READONLY:
	      break;
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		break;
	    default:
		return EINVAL;
	}
}
*/
