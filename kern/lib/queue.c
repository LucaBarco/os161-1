#include <queue.h>
#include <list.h>
#include <types.h>
#include <lib.h>

struct queue {
    struct list* vals;
};

struct queue*
queue_create(void)
{
    struct queue* q = (struct queue*)kmalloc(sizeof(struct queue));
    q->vals = list_create();
    if (q->vals == NULL) {
        return NULL;
    }
    return q;
}

int
queue_push(struct queue* q, void* newval)
{
    KASSERT(q != NULL);
    return list_push_back(q->vals, newval);
}

void
queue_pop(struct queue* q)
{
    KASSERT(q != NULL);
    list_pop_front(q->vals);
}

void*
queue_front(struct queue* q)
{
    KASSERT(q != NULL);
    return list_front(q->vals);
}

int
queue_isempty(struct queue* q)
{
    KASSERT(q != NULL);
    return list_isempty(q->vals);
}

unsigned int
queue_getsize(struct queue* q)
{
    KASSERT(q != NULL);
    return list_getsize(q->vals);
}

void
queue_destroy(struct queue* q)
{
    if (q != NULL) {
        list_destroy(q->vals);
    }
    kfree(q);
}
