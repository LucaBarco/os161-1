/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <proc.h>
#include <addrspace.h>
#include <vm.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

struct addrspace *
as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		return NULL;
	}
    
    as->segment_index = 0;
	as->page_table = (struct page_table_entry*)alloc_kpages(1);
    as->ignore_permissions = 0;
    
	return as;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}

	int i,j;
	for(i = 0; i < old->segment_index; i++) {
        newas->segment_table[i].start = old->segment_table[i].start;
        newas->segment_table[i].end = old->segment_table[i].end;
        newas->segment_table[i].read = old->segment_table[i].read;
        newas->segment_table[i].write = old->segment_table[i].write;
        newas->segment_table[i].execute = old->segment_table[i].execute;
    }
    
    for(i = 0; i < 1024; i++)
        if(old->page_table[i].valid) {
            newas->page_table[i].index = alloc_kpages(1) >> 12;
            newas->page_table[i].valid = 1;
            for(j = 0; j < 1024; j++)
                if(((struct page_table_entry *)(old->page_table[i].index << 12))->valid) {
                    ((struct page_table_entry *)(newas->page_table[i].index << 12))->index = alloc_kpages(1) >> 12;
                    ((struct page_table_entry *)(newas->page_table[i].index << 12))->valid = 1;
                    memcpy((void*)(((struct page_table_entry *)(newas->page_table[i].index << 12))->index << 12), (void*)(((struct page_table_entry *)(old->page_table[i].index << 12))->index << 12), 4096);
                }
        }

	*ret = newas;
	return 0;
}

void
as_destroy(struct addrspace *as)
{
    int i,j;
    for(i = 0; i < 1024; i++)
        if(as->page_table[i].valid) {
            for(j = 0; j < 1024; j++)
                if(((struct page_table_entry *)(as->page_table[i].index << 12))->valid)
                    free_kpages(((struct page_table_entry *)(as->page_table[i].index << 12))->index << 12);
            free_kpages(as->page_table[i].index << 12);
        }
    free_kpages((vaddr_t)as->page_table);

	kfree(as);
}

void
as_activate(void)
{
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		/*
		 * Kernel thread without an address space; leave the
		 * prior address space in place.
		 */
		return;
	}

	vm_tlbshootdown_all();
}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable)
{
    as->segment_table[as->segment_index].start = vaddr;
    as->segment_table[as->segment_index].end = vaddr+sz;
    as->segment_table[as->segment_index].read = readable;
    as->segment_table[as->segment_index].write = writeable;
    as->segment_table[as->segment_index].execute = executable;
    
    as->segment_index++;
	return 0;
}

int
as_prepare_load(struct addrspace *as)
{
	as->ignore_permissions = 1;
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	as->ignore_permissions = 0;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;
    
	as_define_region(as, *stackptr, 0, 1, 1, 0);
    
	return 0;
}

