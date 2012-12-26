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
#include "math.h"

struct kmem_list initkmem[NUM_INIT_LISTS];
struct list_head cache_chain;

unsigned int * slab_bufctl(struct slab *slabp)
{
    return (unsigned int *)(slabp + 1);
}

unsigned int obj_to_index(struct kmem_cache *cachep, struct slab *slabp, void *objp)
{
    return ((objp - slabp->s_mem)/cachep->obj_size);
}

void * index_to_obj(struct kmem_cache *cachep, struct slab *slabp, unsigned int free)
{
    return (slabp->s_mem + cachep->obj_size * free);
}

void * slab_get_obj(struct kmem_cache *cachep, struct slab *slabp)
{
    if(cachep == NULL || slabp == NULL)
        return NULL;
    void *objp = index_to_obj(cachep, slabp, slabp->free);
    int next = slab_bufctl(slabp)[slabp->free];
    slabp->inuse++;
    slabp->free = next;

    return objp;
}

void slab_free_obj(struct kmem_cache *cachep, struct slab *slabp, void *objp)
{
    unsigned int objnr = obj_to_index(cachep, slabp, objp);
    if(objnr > cachep->obj_num)
        return;
    slab_bufctl(slabp)[objnr] = slabp->free;
    slabp->free = objnr;
    slabp->inuse--;
}

/* alloc slab pages default for 4 pages */
inline void * slab_alloc()
{
    unsigned long address = get_free_pages(DEFAULT_SLAB_PAGES);

    return (void *)address;
}

/* estimate the slab parameters */
inline int slab_estimate(const int obj_size, int *obj_num)
{
    *obj_num = ((power(DEFAULT_SLAB_PAGES) * PAGE_SIZE) - sizeof(struct slab))/obj_size;

    return 0;
}

/* init slab 
 * struct slab + obj_num * (unsigned int)*/
int slab_init(struct kmem_cache *cachep, struct slab *slabp, void *args)
{
    int i = 0;
    void *ptr = args;
    slabp->free = cachep->obj_num;
    slabp->inuse = 0;
    INIT_LIST_HEAD(&(slabp->list));
    ptr += sizeof(cachep->obj_num * sizeof(unsigned int));
    slabp->s_mem = ptr;
    for(i = 0;i < cachep->obj_num; ++i)
    {
        slab_bufctl(slabp)[i] = i + 1;
    }
    slab_bufctl(slabp)[i - 1] =  BUFCTL_END;

    return 0;
}

/* add a slab to kmem_cache */
int kmem_add_slab(struct kmem_cache *cachep, struct slab *slabp)
{
    if(cachep == NULL || slabp == NULL)
        return -1;

    /*cachep->obj_num += slabp->free;*/
    cachep->nr_frees += slabp->free;
    list_add_tail(&(slabp->list), &(cachep->lists.free));

    return 0;
}

/* create a slab and add to kmem_cache */
int cache_grow(struct kmem_cache *cachep, const int obj_size)
{
    if(cachep == NULL || obj_size == 0)
    {
        return -1;
    }
    void *ptr;
    struct slab *slabp = NULL;
    /*unsigned long slab_size;*/
    int obj_num = 0;

    ptr = slab_alloc();

    /*slab_size = sizeof(struct slab) + obj_size * obj_num;*/

    slabp = (struct slab *)ptr;
    slab_estimate(obj_size, &obj_num);

    ptr += sizeof(struct slab);
    slab_init(cachep, slabp, ptr);
    
    kmem_add_slab(cachep, slabp);

    return 0;
}

int print_kmem_info(struct kmem_cache *cachep)
{
    if(cachep == NULL)
        return -1;

    printk("name = %s.", cachep->name);
    printk("obj_size = %d.", cachep->obj_size);
    /*printk("obj_num = %d.", cachep->objobj__num);*/
    printk("nr_frees = %d.\n", cachep->nr_frees);

    return 0;
}

void printk_kmem_chain()
{
    struct kmem_cache *cachep = NULL;
    struct list_head *head, *pos, *n;

    head = &(cache_chain);

    list_for_each_safe(pos, n, head)
    {
        cachep = list_entry(pos, struct kmem_cache, next);
        print_kmem_info(cachep);
    }
}

/* init kmem_list  */
int kmem_list_init(struct kmem_list *parent)
{
    INIT_LIST_HEAD(&(parent->partial));
    INIT_LIST_HEAD(&(parent->full));
    INIT_LIST_HEAD(&(parent->free));
    /*parent->free_obj = 0;*/

    return 0;
}

/* the slab system start  */
int kmem_cache_init()
{
    /*int i = 0;*/
    /*for(i  = 0;i < NUM_INIT_LISTS; ++i)*/
    /*kmem_list_init(&(initkmem[i]));*/

    INIT_LIST_HEAD(&cache_chain);

    return 0;
}

int kmem_chain_add(struct kmem_cache *cachep)
{
    if(cachep == NULL)
        return -1;

    struct list_head *head = NULL, *pos, *n;
    struct kmem_cache *item_cachep = NULL;

    head = &(cache_chain);

    if(list_empty_careful(head))
    {
        list_add(&(cachep->next),&(cache_chain));
    }
    else
    {
        list_for_each_safe(pos, n, head)
        {
            item_cachep = list_entry(pos, struct kmem_cache, next);
            if(cachep->obj_size < item_cachep->obj_size)
            {
                __list_add(&(cachep->next), pos->prev, pos);
                return 0;
            }
            if(pos->next == head)
            {
                __list_add(&(cachep->next), pos, n);
                return 0;
            }
        }
    }

    return -1;
}

/* alloc kmem_cache */
void * kmem_cache_alloc()
{
    /*get a free page from buddy list
     * kmem_cache struct must in one page*/
    if(sizeof(struct kmem_cache)> PAGE_SIZE)
        return NULL;

    int order = (sizeof(struct kmem_cache) >>PAGE_SHIFT);
    unsigned long address = get_free_pages(order);

    return (void *)address;
}

/* just only create kmem_cache 
 * and add to cache_chain*/
int kmem_cache_create(char *name, size_t obj_size, unsigned long flags)
{
    void *ptr;
    struct kmem_cache *cachep = NULL;
    /*unsigned long cache_size = 0;*/

    if(name == NULL || obj_size == 0)
        return -1;

    ptr = kmem_cache_alloc();

    if(ptr == NULL)
        return -2;

    cachep = (struct kmem_cache *)ptr;
    strncpy(cachep->name, name, sizeof(cachep->name));
    cachep->obj_size = obj_size;
    cachep->nr_frees = 0;
    cachep->obj_num = 0;
    cachep->flags = flags;

    kmem_list_init(&(cachep->lists));
    cache_grow(cachep, obj_size);
    kmem_chain_add(cachep);
    /*print_kmem_info(cachep);*/

    return 0;
}

unsigned int kmem_cache_size(struct kmem_cache *cachep)
{
    return cachep->obj_size;
}

unsigned int kmem_cache_num(struct kmem_cache *cachep)
{
    return cachep->obj_num;
}

const char * kmem_cache_name(struct kmem_cache *cachep)
{
    return cachep->name;
}

unsigned int kmem_cache_frees(struct kmem_cache *cachep)
{
    return cachep->nr_frees;
}
