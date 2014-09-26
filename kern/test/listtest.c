#include <list.h>
#include <types.h>
#include <lib.h>
#include <test.h>

#define TESTSIZE 133

int
listtest(int nargs, char **args)
{
    (void)nargs;
    (void)args;

    kprintf("Beginning list test...\n");

    struct list* newlist;
    newlist = list_create();
    KASSERT(newlist != NULL);
    KASSERT(list_getsize(newlist) == 0);
    KASSERT(list_isempty(newlist));

    int i;
    int* elem;
    /* push back TESTSIZE number of elements */
    for (i = 0; i < TESTSIZE; ++i) {
        elem = (int*)kmalloc(sizeof(int));
        *elem = i;
        list_push_back(newlist, (void*) elem);
    }
    KASSERT(list_getsize(newlist) == TESTSIZE);
    KASSERT(!list_isempty(newlist));

    /* pop front TESTSIZE number of elements */
    for (i = 0; i < TESTSIZE; ++i) {
        elem = (int*)list_front(newlist);
        KASSERT(*elem == i);
        list_pop_front(newlist);
        kfree(elem);
    }
    KASSERT(list_getsize(newlist) == 0);
    KASSERT(list_isempty(newlist));

    /* push back TESTSIZE number of elements */
    for (i = 0; i < TESTSIZE; ++i) {
        elem = (int*)kmalloc(sizeof(int));
        *elem = i;
        list_push_back(newlist, (void*) elem);
    }
    KASSERT(list_getsize(newlist) == TESTSIZE);
    KASSERT(!list_isempty(newlist));

    /* pop front TESTSIZE number of elements */
    for (i = 0; i < TESTSIZE; ++i) {
        elem = (int*)list_front(newlist);
        KASSERT(*elem == i);
        list_pop_front(newlist);
        kfree(elem);
    }
    KASSERT(list_getsize(newlist) == 0);
    KASSERT(list_isempty(newlist));

    list_destroy(newlist);

    kprintf("List test complete\n");

    return 0;
}
