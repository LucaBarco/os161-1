/*
 * Copyright (c) 2009
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

#ifndef _PROCLIST_H_
#define _PROCLIST_H_


struct proc;	/* from <proc.h> */

/*
 * AmigaOS-style linked list of threads.
 *
 * The two threadlistnodes in the threadlist structure are always on
 * the list, as bookends; this removes all the special cases in the
 * list handling code. However, note how THREADLIST_FOREACH works: you
 * iterate by starting with tl_head.tln_next, and stop when
 * itervar->tln_next is null, not when itervar itself becomes null.
 *
 * ->tln_self always points to the process that contains the
 * proclistnode. We could avoid this if we wanted to instead use
 *
 *    (struct ptoc *)((char *)node - offsetof(struct proc, t_listnode))
 *
 * to get the process pointer. But that's gross.
 */

struct proclistnode {
	struct proclistnode *tln_prev;
	struct proclistnode *tln_next;
	struct proc *tln_self;
};

struct proclist {
	struct proclistnode tl_head;
	struct proclistnode tl_tail;
	unsigned tl_count;
};

/* Initialize and clean up a process list node. */
void proclistnode_init(struct proclistnode *tln, struct proc *self);
void proclistnode_cleanup(struct proclistnode *tln);

/* Initialize and clean up a process list. Must be empty at cleanup. */
void proclist_init(struct proclist *tl);
void proclist_cleanup(struct proclist *tl);

/* Check if it's empty */
bool proclist_isempty(struct proclist *tl);

/* Add and remove: at ends */
void proclist_addhead(struct proclist *tl, struct proc *t);
void proclist_addtail(struct proclist *tl, struct proc *t);
struct thread *threadlist_remhead(struct proclist *tl);
struct thread *threadlist_remtail(struct proclist *tl);

/* Add and remove: in middle. (TL is needed to maintain ->tl_count.) */
void proclist_insertafter(struct proclist *tl,
			    struct proc *onlist, struct proc *addee);
void proclist_insertbefore(struct proclist *tl,
			     struct proc *addee, struct proc *onlist);
void proclist_remove(struct proclist *tl, struct proc *t);

/* Iteration; itervar should previously be declared as (struct thread *) */
#define PROCLIST_FORALL(itervar, tl) \
	for ((itervar) = (tl).tl_head.tln_next->tln_self; \
	     (itervar) != NULL; \
	     (itervar) = (itervar)->t_listnode.tln_next->tln_self)

#define PROCLIST_FORALL_REV(itervar, tl) \
	for ((itervar) = (tl).tl_tail.tln_prev->tln_self; \
	     (itervar) != NULL; \
	     (itervar) = (itervar)->t_listnode.tln_prev->tln_self)


#endif /* _PROCLIST_H_ */
