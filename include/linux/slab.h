#ifndef     _SLAB_H_
#define     _SLAB_H_

#include "list.h"
#include "asm-i386/page.h"

#define     BYTES_PER_WORD      sizeof(void *)
#define     DEFAULT_SLAB_PAGES   4
#define     MAX_SLAB_PAGES       8

typedef unsigned int kmem_bufctl_t;

#define     BUFCTL_END      (((kmem_bufctl_t)(~0U))-0)
#define     BUFCTL_FREE     (((kmem_bufctl_t)(~0U))-1)
#define     SLAB_LIMIT      (((kmem_bufctl_t)(~0U))-2)

struct slab
{
    struct list_head list; 
    //unsigned int obj;   [> object in slab <] 
    void * s_mem;   /* the first object of slab */
    unsigned int free;  /* the next free object of slab */
    unsigned int inuse;  /* number of objs inused in slab */
};

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
    unsigned int nr_pages;

    int obj_size;   /* each slab object size */
    int obj_num;        /* each slab object number */
    unsigned long obj_offset;   /* object offset */

    //void (*ctor)(void *obj);  [> constructor of object <]
    //void (*dtor)(void *obj);  [> destructor of object <]

    char name[16];      /* name */
    struct list_head next;  /* next */
    struct kmem_list lists;     /* lists */
}Kmem_cache;

/* mm node */
#define     MAX_NUMNODES 1
#define     NUM_INIT_LISTS  3*MAX_NUMNODES

extern struct kmem_list  initkmem[NUM_INIT_LISTS];
/* kmem_cache chain */
extern struct list_head cache_chain;
extern struct kmem_cache *vma_cachep;
extern struct kmem_cache *tsk_cachep;
extern struct kmem_cache *thread_union_cachep;
extern struct kmem_cache *inode_cachep;
extern struct kmem_cache *file_cachep;
//extern struct kmem_cache *dentry_cachep;

int kmem_cache_init();
struct kmem_cache * kmem_cache_create(char *name, size_t obj_size, unsigned long flags);
struct slab * kmem_get_slab(struct kmem_cache *cachep);
void * kmem_get_obj(struct kmem_cache *cachep);
int kmem_free_obj(struct kmem_cache *cachep, void *objp);
void * slab_get_obj(struct kmem_cache *cachep, struct slab *slabp);
int slab_free_obj(struct kmem_cache *cachep, struct slab *slabp, void *objp);
int print_kmem_info(struct kmem_cache *cachep);
void print_kmem_chain();
int print_slab_info(struct slab *slabp);
int init_kmem_cache();

#endif
