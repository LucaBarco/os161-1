
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <mips/tlb.h>
#include <spl.h>
#include <vm.h>

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
