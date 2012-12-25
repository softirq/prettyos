#ifndef     _SLAB_H_
#define     _SLAB_H_

#include "list.h"
#include "asm-i386/page.h"

#define     BYTES_PER_WORD      sizeof(void *)

typedef struct slab
{
    struct list_head list; 
    unsigned int obj;   /* object in slab */ 
    void * s_mem;   /* the first object of slab */
    unsigned int free;  /* the next free object of slab */
    int inuse;  /* number of objs inused in slab */
}Slab;

struct kmem_list
{
    struct list_head partial;   /* partial used */
    struct list_head full;      /* full used */
    struct list_head free;      /* free */
};

typedef struct kmem_cache
{
    unsigned int flags;     /* flags */
    unsigned int nr_frees;  /* numbers of free */
    unsigned long free_limit;   /* free number limit */

    int obj_size;   /* object size */
    int num;        /* object number */
    unsigned long obj_offset;   /* object offset */

    void (*ctor)(void *obj);  /* constructor of object */
    void (*dtor)(void *obj);  /* destructor of object */

    char name[16];      /* name */
    struct list_head next;  /* next */
    struct kmem_list *list;     /* lists */
}Kmem_cache;

/* mm node */
#define     MAX_NUMNODES 1
#define     NUM_INIT_LISTS  3*MAX_NUMNODES

extern struct kmem_list  initkmem[NUM_INIT_LISTS];
/* kmem_cache chain */
extern struct list_head cache_chain;


#endif
