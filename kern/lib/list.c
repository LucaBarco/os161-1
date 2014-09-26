#include <list.h>
#include <types.h>
#include <lib.h>

struct listnode {
    void* val;
    struct listnode* next;
};

struct list {
    struct listnode* head;
    struct listnode* tail;
    unsigned int size;
};

/* Allocates and returns a list node object containing given element */
struct listnode* listnode_create(void* newval);

struct listnode*
listnode_create(void* newval)
{
    struct listnode* newnode;
    
    newnode = (struct listnode*)kmalloc(sizeof(struct listnode));
    if (newnode == NULL) {
        return NULL;
    }
    newnode->val = newval;
    newnode->next = NULL;

    return newnode;
}

struct list*
list_create(void)
{
    struct list* newlist;

    newlist = (struct list*)kmalloc(sizeof(struct list));
    if (newlist == NULL) {
        return NULL;
    }

    newlist->head = NULL;
    newlist->tail = NULL;
    newlist->size = 0;

    return newlist;
}

void
list_push_back(struct list* lst, void* newval)
{
    KASSERT(lst != NULL);

    struct listnode* newnode = listnode_create(newval);
    if (newnode == NULL) {
        return;
    }

    if (lst->size == 0) {
        lst->head = newnode;
    } else {
        lst->tail->next = newnode;
    }
    lst->tail = newnode;

    ++lst->size;
}

void
list_pop_front(struct list* lst)
{
    KASSERT(lst != NULL);

    if (lst->size == 0) {
        return;
    }

    lst->head = lst->head->next;
    --lst->size;

    if (lst->size == 0) {
        lst->tail = NULL;
    }
}

void*
list_front(struct list* lst)
{
    KASSERT(lst != NULL);

    if (lst->size == 0) {
        return NULL;
    }

    return lst->head->val;
}

int
list_isempty(struct list* lst)
{
    KASSERT(lst != NULL);

    return (lst->size == 0);
}

unsigned int
list_getsize(struct list* lst)
{
    KASSERT(lst != NULL);

    return (lst->size);
}

void
list_destroy(struct list* lst)
{
    if (lst != NULL) {
        struct listnode* p = lst->head;
        struct listnode* q;
        /* frees every listnode object in the list */
        while (p != NULL) {
            q = p->next;
            kfree(p);
            p = q;
        }
    }
    /* frees the list object itself */
    kfree(lst);
}

