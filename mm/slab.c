/* slab system */
#include "type.h"
#include "const.h"
#include "string.h"
#include "stdlib.h"
#include "list.h"
#include "panic.h"
#include "wait.h"
#include "printf.h"
#include "mm.h"
#include "list.h"

struct kmem_list initkmem[NUM_INIT_LISTS];
struct list_head cache_chain;


int kmem_list_init(struct kmem_list *parent)
{
    INIT_LIST_HEAD(&(parent->partial));
    INIT_LIST_HEAD(&(parent->full));
    INIT_LIST_HEAD(&(parent->free));
    parent->free_obj = 0;

    return 0;
}

int kmem_cache_init()
{
    int i = 0;
    for(i  = 0;i < NUM_INIT_LISTS; ++i)
        kmem_list_init(&(initkmem[i]));

    INIT_LIST_HEAD(&cache_chain);

    return 0;
}

void* kmem_cache_alloc()
{
    /*get a free page from buddy list*/
    if(sizeof(struct kmem_cache) > PAGE_SIZE)
        return NULL;

    struct page *page = get_free_pages(0);

    return (void *)page;
}

int kmem_cache_create(char *name, size_t size, unsigned long flags, void (*ctor)(void *))
{
    void *ret;
    struct kmem_cache *cachep;
    if(name == NULL || ctor == NULL || size == 0)
        return -1;

    ret = kmem_cache_alloc();
    if(ret == NULL)
        return -2;

    cachep = (struct kmem_cache *)ret;
    strncpy(cachep->name, name, sizeof(cachep->name));
    cachep->obj_size = size;

    return 0;
}
