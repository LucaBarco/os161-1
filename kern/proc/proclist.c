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

/*
 * Process list functions, rather dull.
 */

#include <types.h>
#include <lib.h>
#include <proc.h>
#include <proclist.h>

void
proclistnode_init(struct proclistnode *tln, struct proc *t)
{
	DEBUGASSERT(tln != NULL);
	KASSERT(t != NULL);

	tln->tln_next = NULL;
	tln->tln_prev = NULL;
	tln->tln_self = t;
}

void
proclistnode_cleanup(struct proclistnode *tln)
{
	DEBUGASSERT(tln != NULL);

	KASSERT(tln->tln_next == NULL);
	KASSERT(tln->tln_prev == NULL);
	KASSERT(tln->tln_self != NULL);
}

void
proclist_init(struct proclist *tl)
{
	DEBUGASSERT(tl != NULL);

	tl->tl_head.tln_next = &tl->tl_tail;
	tl->tl_head.tln_prev = NULL;
	tl->tl_tail.tln_next = NULL;
	tl->tl_tail.tln_prev = &tl->tl_head;
	tl->tl_head.tln_self = NULL;
	tl->tl_tail.tln_self = NULL;
	tl->tl_count = 0;
}

void
proclist_cleanup(struct proclist *tl)
{
	DEBUGASSERT(tl != NULL);
	DEBUGASSERT(tl->tl_head.tln_next == &tl->tl_tail);
	DEBUGASSERT(tl->tl_head.tln_prev == NULL);
	DEBUGASSERT(tl->tl_tail.tln_next == NULL);
	DEBUGASSERT(tl->tl_tail.tln_prev == &tl->tl_head);
	DEBUGASSERT(tl->tl_head.tln_self == NULL);
	DEBUGASSERT(tl->tl_tail.tln_self == NULL);

	KASSERT(proclist_isempty(tl));
	KASSERT(tl->tl_count == 0);

	/* nothing (else) to do */
}

bool
proclist_isempty(struct proclist *tl)
{
	DEBUGASSERT(tl != NULL);

	return (tl->tl_count == 0);
}

////////////////////////////////////////////////////////////
// internal

/*
 * Do insertion. Doesn't update tl_count.
 */
static
void
proclist_insertafternode(struct proclistnode *onlist, struct proc *t)
{
	struct proclistnode *addee;

	addee = &t->t_listnode;

	DEBUGASSERT(addee->tln_prev == NULL);
	DEBUGASSERT(addee->tln_next == NULL);

	addee->tln_prev = onlist;
	addee->tln_next = onlist->tln_next;
	addee->tln_prev->tln_next = addee;
	addee->tln_next->tln_prev = addee;
}

/*
 * Do insertion. Doesn't update tl_count.
 */
static
void
proclist_insertbeforenode(struct proc *t, struct proclistnode *onlist)
{
	struct proclistnode *addee;

	addee = &t->t_listnode;

	DEBUGASSERT(addee->tln_prev == NULL);
	DEBUGASSERT(addee->tln_next == NULL);

	addee->tln_prev = onlist->tln_prev;
	addee->tln_next = onlist;
	addee->tln_prev->tln_next = addee;
	addee->tln_next->tln_prev = addee;
}

/*
 * Do removal. Doesn't update tl_count.
 */
static
void
proclist_removenode(struct proclistnode *tln)
{
	DEBUGASSERT(tln != NULL);
	DEBUGASSERT(tln->tln_prev != NULL);
	DEBUGASSERT(tln->tln_next != NULL);

	tln->tln_prev->tln_next = tln->tln_next;
	tln->tln_next->tln_prev = tln->tln_prev;
	tln->tln_prev = NULL;
	tln->tln_next = NULL;
}

////////////////////////////////////////////////////////////
// public

void
proclist_addhead(struct proclist *tl, struct proc *t)
{
	DEBUGASSERT(tl != NULL);
	DEBUGASSERT(t != NULL);

	proclist_insertafternode(&tl->tl_head, t);
	tl->tl_count++;
}

void
proclist_addtail(struct proclist *tl, struct proc *t)
{
	DEBUGASSERT(tl != NULL);
	DEBUGASSERT(t != NULL);

	proclist_insertbeforenode(t, &tl->tl_tail);
	tl->tl_count++;
}

struct proc *
proclist_remhead(struct proclist *tl)
{
	struct proclistnode *tln;

	DEBUGASSERT(tl != NULL);

	tln = tl->tl_head.tln_next;
	if (tln->tln_next == NULL) {
		/* list was empty  */
		return NULL;
	}
	proclist_removenode(tln);
	DEBUGASSERT(tl->tl_count > 0);
	tl->tl_count--;
	return tln->tln_self;
}

struct proc *
proclist_remtail(struct proclist *tl)
{
	struct proclistnode *tln;

	DEBUGASSERT(tl != NULL);

	tln = tl->tl_tail.tln_prev;
	if (tln->tln_prev == NULL) {
		/* list was empty  */
		return NULL;
	}
	proclist_removenode(tln);
	DEBUGASSERT(tl->tl_count > 0);
	tl->tl_count--;
	return tln->tln_self;
}

void
proclist_insertafter(struct proclist *tl,
		       struct proc *onlist, struct proc *addee)
{
	proclist_insertafternode(&onlist->t_listnode, addee);
	tl->tl_count++;
}

void
proclist_insertbefore(struct proclist *tl,
			struct proc *addee, struct proc *onlist)
{
	proclist_insertbeforenode(addee, &onlist->t_listnode);
	tl->tl_count++;
}

void
proclist_remove(struct proclist *tl, struct proc *t)
{
	proclist_removenode(&t->t_listnode);
	DEBUGASSERT(tl->tl_count > 0);
	tl->tl_count--;
}
