
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <mips/tlb.h>
#include <spl.h>
#include <vm.h>
#include <addrspace.h>
#include <current.h>
#include <proc.h>

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

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
    if (curproc == NULL) {
        /*
         * No process. This is probably a kernel fault early
         * in boot. Return EFAULT so as to panic instead of
         * getting into an infinite faulting loop.
         */
        return EFAULT;
    }

    struct addrspace * as = proc_getas();
    if (as == NULL) {
        /*
         * No address space set up. This is probably also a
         * kernel fault early in boot.
        */
        return EFAULT;
    }
    
    int i;
    for(i=0;i<4;i++) {
        if(faultaddress >= as->segment_table[i].start && faultaddress < as->segment_table[i].end) {
            if(!as->ignore_permissions) {
                switch (faulttype) {
                    //should probably throw an error somehow instead of kassert
                    case VM_FAULT_READONLY:
                        KASSERT(as->segment_table[i].write && as->page_table[faultaddress >> 12].valid);
                        break;
                    case VM_FAULT_READ:
                        KASSERT(as->segment_table[i].read);
                        break;
                    case VM_FAULT_WRITE:
                        KASSERT(as->segment_table[i].write);
                        break;
                    default:
                        return EINVAL;
                }
            }
        }
    }
    
    if(!as->page_table[faultaddress >> 12].valid) {
        as->page_table[faultaddress >> 12].index = alloc_kpages(1);
        //STEP 3: put reverse lookup entry in coremap
    }
    
    unsigned int flags = TLBLO_VALID;
    switch (faulttype) {
        case VM_FAULT_READONLY:
            i = tlb_probe(faultaddress & TLBHI_VPAGE, 0);
            KASSERT(i >= 0);
            tlb_write(faultaddress & TLBHI_VPAGE, (as->page_table[faultaddress >> 12].index << 12)|TLBLO_DIRTY|TLBLO_VALID, i);
            break;
        case VM_FAULT_WRITE:
            flags = flags | TLBLO_DIRTY;
        case VM_FAULT_READ:
            i = tlb_probe(faultaddress & TLBHI_VPAGE, 0);
            if(i >= 0)
                tlb_write(faultaddress & TLBHI_VPAGE, (as->page_table[faultaddress >> 12].index << 12)|flags, i);
            else
                tlb_random(faultaddress & TLBHI_VPAGE, (as->page_table[faultaddress >> 12].index << 12)|flags);
            break;
        default:
            return EINVAL;
    }
    return 0;
}

