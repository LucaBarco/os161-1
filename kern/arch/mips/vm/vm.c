
#include <mips/tlb.h>
#include <spinlock.h>
#include <spl.h>
#include <types.h>
#include <vm.h>

void
vm_tlbshootdown_all(void)
{
    spl = splhigh();
    
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
